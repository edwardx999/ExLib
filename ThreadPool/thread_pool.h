#ifndef EXLIB_THREAD_POOL_H
#define EXLIB_THREAD_POOL_H
#include <functional>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <tuple>
#include <queue>
#include <memory>
#include <type_traits>
#include <thread>
#ifdef _MSVC_LANG
#define _EXLIB_THREAD_POOL_HAS_CPP_20 _MSVC_LANG>202000l
#define _EXLIB_THREAD_POOL_HAS_CPP_17 _MSVC_LANG>201700l
#else
#define _EXLIB_THREAD_POOL_HAS_CPP_20 __cplusplus>202000l
#define _EXLIB_THREAD_POOL_HAS_CPP_17 __cplusplus>201700l
#endif
namespace exlib {

	namespace detail {
		template<typename Func,typename Tuple,size_t... Indices>
		decltype(auto) invoke(Func&& f,Tuple&& tpl,std::index_sequence<Indices...>)
		{
			using std::get;
			return std::forward<Func>(f)(get<Indices>(std::forward<Tuple>(tpl))...);
		}

#if _EXLIB_THREAD_POOL_HAS_CPP_17

		template<typename... Args>
		using no_rvalue_references=std::conjunction<std::negation<std::is_rvalue_reference<Args>>...>;
#else
		template<typename... Args>
		struct no_rvalue_references;

		template<>
		struct no_rvalue_references<>:std::true_type {};

		template<typename T,typename... Rest>
		struct no_rvalue_references<T,Rest...>:std::integral_constant<bool,!std::is_rvalue_reference<T>::value&&no_rvalue_references<Rest...>::value> {};
#endif

#if _EXLIB_THREAD_POOL_HAS_CPP_20
		template<typename T>
		using std::remove_cv_ref_t;
#else
		template<typename T>
		using remove_cv_ref_t=typename std::remove_cv<typename std::remove_reference<T>::type>::type;
#endif
	}
	struct delay_start_t {};
	/*
		Thread pool where each task is given the same arguments as the types given.
		Recommended using thread_pool if using a thread pool multiple times to avoid code bloat/
	*/
	template<typename... Args>
	class thread_pool_a {
		static_assert(detail::no_rvalue_references<Args...>::value,"RValue References not allowed as start arguments");
		using TaskInput=std::tuple<Args...>;
		using IdxSeq=std::make_index_sequence<sizeof...(Args)>;
		struct job {
			virtual void operator()(TaskInput const& input) const=0;
			virtual ~job()=default;
		};
		template<typename BaseFunc>
		struct job_impl:job {
			BaseFunc task;
			template<typename F>
			job_impl(F&& f):task(std::forward<F>(f)) {}
			void operator()(TaskInput const& input) const
			{
				detail::invoke(task,input,IdxSeq{});
			}
		};
		template<typename Task>
		static std::unique_ptr<job> make_task(Task&& the_task)
		{
			return std::unique_ptr<job>(new job_impl<detail::remove_cv_ref_t<Task>>(std::forward<Task>(the_task)));
		}
		void task_loop()
		{
			while(true)
			{
				std::unique_ptr<job> task;
				size_t jobs_left;
				{
					if(!_running)
					{
						return; //don't bother locking if not running
					}
					std::unique_lock<std::mutex> lock(_mtx);
					while(true)
					{
						if(!_running)
						{
							return;
						}
						jobs_left=_jobs.size();
						if(_active&&jobs_left)
						{
							task=std::move(_jobs.front());
							_jobs.pop();
							break;
						}
						_signal_start.wait(lock);
					}
				}
				(*task)(_input);
				if(jobs_left==1)
				{
					_jobs_done.notify_one();
				}
			}
		}
	public:
		template<typename... T>
		explicit thread_pool_a(size_t num_threads,T&&... args):_workers(num_threads),_input(std::forward<T>(args)...)
		{
			start();
		}

		template<typename... T>
		explicit thread_pool_a(size_t num_threads,delay_start_t,T&&... args):_workers(num_threads),_input(std::forward<T>(args)...)
		{}

		thread_pool_a():thread_pool_a([]()->size_t
		{
			auto const nt=std::thread::hardware_concurrency();
			if(nt)
			{
				return nt;
			}
			return 1;
		}())
		{}

		template<typename... TplArgs>
		void set_args(TplArgs&&... args)
		{
			_input=TaskInput(*this,std::forward<TplArgs>(args)...);
		}

		/*
			Starts threads.
		*/
		void start()
		{
			_running=true;
			_active=true;

			for(auto& worker:_workers)
			{
				worker=std::thread(&thread_pool_a::task_loop,this);
			}
		}


		/*
			Makes threads look for tasks. Useful after stop() has been called to reactivate job search.
		*/
		void reactivate()
		{
			_active=true;
			_signal_start.notify_all();
		}

		/*
			Makes threads look for tasks. Useful after stop() has been called to reactivate job search.
		*/
		template<typename... Args>
		void reactivate(Args&&... args)
		{
			set_args(std::forward<Args>(args));
			_active=true;
			_signal_start.notify_all();
		}

		/*
			Waits for all jobs to be finished. If thread pool is not active, does nothing
		*/
		void wait()
		{
			if(_active)
			{
				std::unique_lock<std::mutex> lock(_mtx);
				_jobs_done.wait(lock,[&jobs=_jobs]
				{
					return jobs.empty();
				});
			}
		}

		/*
			Waits for all jobs to be finished. If thread pool is not active, does nothing and returns false. Returns false if timeout expired.
		*/
		template< class Rep,class Period >
		bool wait_for(const std::chrono::duration<Rep,Period>& rel_time)
		{
			if(_active)
			{
				std::unique_lock<std::mutex> lock(_mtx);
				return _jobs_done.wait_for(lock,rel_time,[&jobs=_jobs]
				{
					return jobs.empty();
				});
			}
			return false;
		}

		/*
			Waits for all jobs to be finished. If thread pool is not active, does nothing and returns false. Returns false if timeout expired.
		*/
		template< class Rep,class Period >
		bool wait_until(const std::chrono::duration<Rep,Period>& rel_time)
		{
			if(_active)
			{
				std::unique_lock<std::mutex> lock(_mtx);
				return _jobs_done.wait_until(lock,rel_time,[&jobs=_jobs]
				{
					return jobs.empty();
				});
			}
			return false;
		}

		/*
			Makes threads stop looking for jobs.
		*/
		void stop()
		{
			_active=false;
		}

		/*
			Makes threads stop looking for jobs and ends threads.
		*/
		void terminate()
		{
			stop();
			_running=false;
			_signal_start.notify_all();
			join();
		}

		/*
			Waits for all jobs to finish and ends threads.
		*/
		void join()
		{
			wait();
			_active=false;
			_running=false;
			_signal_start.notify_all();
			for(auto& thread:_workers)
			{
				if(thread.joinable())
				{
					thread.join();
				}
			}
		}

		bool running() const
		{
			return _running;
		}

		bool active() const
		{
			return _active;
		}

		~thread_pool_a()
		{
			join();
		}

		template<typename Task>
		void push_back_no_sync(Task&& task)
		{
			_jobs.push(make_task(std::forward<Task>(task)));
		}

		template<typename FirstTask,typename... Rest>
		void push_back_no_sync(FirstTask&& first,Rest&&... rest)
		{
			push_back_no_sync(std::forward<FirstTask>(first));
			push_back_no_sync(std::forward<Rest>(rest)...);
		}

		template<typename... Tasks>
		void push_back(Tasks&&... tasks)
		{
			std::lock_guard<std::mutex> guard(_mtx);
			push_back_no_sync(std::forward<Tasks>(tasks)...);
			notify_count(sizeof...(Tasks));
		}

		template<typename Iter>
		size_t append_no_sync(Iter begin,Iter end,std::random_access_iterator_tag)
		{
			size_t count=end-begin;
			for(;begin!=end;++begin)
			{
				push_back_no_sync(std::forward<decltype(*begin)>(*begin));
			}
			return count;
		}
		template<typename Iter>
		size_t append_no_sync(Iter begin,Iter end,std::input_iterator_tag)
		{
			size_t count=0;
			for(;begin!=end;++begin,++count)
			{
				push_back_no_sync(std::forward<decltype(*begin)>(*begin));
			}
			return count;
		}

		template<typename Iter>
		size_t append(Iter begin,Iter end)
		{
			std::lock_guard<std::mutex> guard(_mtx);
			auto const count=append_no_sync(begin,end,std::iterator_traits<Iter>::iterator_category);
			notify_count(count);
			return count;
		}

	private:
		void notify_count(size_t count)
		{
			if(count==1)
			{
				_signal_start.notify_one();
			}
			else
			{
				_signal_start.notify_all();
			}
		}
		std::mutex _mtx;
		std::condition_variable _signal_start;
		std::condition_variable _jobs_done;
		std::queue<std::unique_ptr<job>> _jobs;
		std::vector<std::thread> _workers;
		//whether threads are running
		std::atomic<bool> _running;
		//whether threads are actively looking for jobs
		std::atomic<bool> _active;
		TaskInput _input;
	};

	using thread_pool=thread_pool_a<>;
}
#endif