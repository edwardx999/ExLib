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
#define _EXLIB_THREAD_POOL_HAS_CPP_14 _MSVC_LANG>201400l
#else
#define _EXLIB_THREAD_POOL_HAS_CPP_20 __cplusplus>202000l
#define _EXLIB_THREAD_POOL_HAS_CPP_17 __cplusplus>201700l
#define _EXLIB_THREAD_POOL_HAS_CPP_14 __cplusplus>201400l
#endif
#if _EXLIB_THREAD_POOL_HAS_CPP_17
#define _EXLIB_THREAD_POOL_NODISCARD [[nodiscard]]
#else
#define _EXLIB_THREAD_POOL_NODISCARD
#endif
namespace exlib {

	namespace detail {

		template<typename T>
		struct wrap_reference {
			using type=T;
		};

		template<typename T>
		struct wrap_reference<T&> {
			using type=std::reference_wrapper<T>;
		};

		template<typename T>
		struct wrap_reference<T const&> {
			using type=std::reference_wrapper<T const>;
		};

		template<typename T>
		using wrap_reference_t=typename wrap_reference<T>::type;

#if _EXLIB_THREAD_POOL_HAS_CPP_14
		using std::index_sequence;

		using std::make_index_sequence;
#else
		template<size_t... Is>
		struct index_sequence {};

		template<typename T,typename U>
		struct concat_index_sequence;

		template<size_t... I,size_t... J>
		struct concat_index_sequence<index_sequence<I...>,index_sequence<J...>> {
			using type=index_sequence<I...,J...>;
		};

		template<typename T,typename U>
		using concat_index_sequence_t=typename concat_index_sequence<T,U>::type;

		template<size_t I,size_t J,bool adjacent=I+1==J>
		struct make_index_sequence_h {
			static constexpr size_t H=I+(J-I)/2;
			using type=concat_index_sequence_t<typename make_index_sequence_h<I,H>::type,typename make_index_sequence_h<H,J>::type>;
		};

		template<size_t I>
		struct make_index_sequence_h<I,I,false> {
			using type=index_sequence<>;
		};

		template<size_t I,size_t J>
		struct make_index_sequence_h<I,J,true> {
			using type=index_sequence<I>;
		};

		template<size_t N>
		using make_index_sequence=typename make_index_sequence_h<0,N>::type;
#endif


#if _EXLIB_THREAD_POOL_HAS_CPP_17
		using std::apply;
#else

		template<typename Func,typename Tuple,size_t... Indices>
		void apply_h(Func&& f,Tuple&& tpl,index_sequence<Indices...>)
		{
			using std::get;
			std::forward<Func>(f)(get<Indices>(std::forward<Tuple>(tpl))...);
		}

		template<typename Func,typename Tuple>
		void apply(Func&& f,Tuple&& tpl)
		{
			apply_h(std::forward<Func>(f),std::forward<Tuple>(tpl),make_index_sequence<std::tuple_size<typename std::remove_reference<Tuple>::type>::value>{});
		}

#endif
		template<typename Func,typename FirstArg,typename Tuple,size_t... Indices>
		void apply_fa_h(Func&& f,FirstArg&& fa,Tuple&& tpl,index_sequence<Indices...>)
		{
			using std::get;
			std::forward<Func>(f)(std::forward<FirstArg>(fa),get<Indices>(std::forward<Tuple>(tpl))...);
		}

		template<typename Func,typename FirstArg,typename Tuple>
		void apply_fa(Func&& f,FirstArg&& fa,Tuple&& tpl)
		{
			apply_fa_h(std::forward<Func>(f),std::forward<FirstArg>(fa),std::forward<Tuple>(tpl),make_index_sequence<std::tuple_size<typename std::remove_reference<Tuple>::type>::value>{});
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
		using std::remove_cvref_t;
#else
		template<typename T>
		using remove_cvref_t=typename std::remove_cv<typename std::remove_reference<T>::type>::type;
#endif
		/*
			The base of the threadpool that does not depend on special arguments.
		*/
		class thread_pool_base {
		public:
			/*
				Makes threads look for tasks. Useful after stop() has been called to reactivate job search.
			*/
			void reactivate()
			{
				_active=true;
				_signal_start.notify_all();
			}
			/*
				Makes threads stop looking for jobs.
			*/
			void stop()
			{
				_active=false;
			}

			/*
				Whether the threads should be running. Not a guarantee that threads are or are not running.
			*/
			bool running() const
			{
				return _running;
			}

			/*
				Whether the threads are looking for tasks.
			*/
			bool active() const
			{
				return _active;
			}
		protected:

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
				std::this_thread::yield();
			}
			void join_all()
			{
				for(auto& thread:this->_workers)
				{
					if(thread.joinable())
					{
						thread.join();
					}
				}
			}
			void internal_stop()
			{
				this->stop();
				this->_jobs_done.notify_one();
			}
			void stop_running()
			{
				this->_running=false;
				this->_signal_start.notify_all();
				this->stop();
			}
			void internal_terminate()
			{
				stop_running();
				_jobs_done.notify_one();
			}

			thread_pool_base(size_t num_threads,bool start):_workers(num_threads),_running(start),_active(start)
			{}
			std::mutex _mtx;
			std::condition_variable _signal_start;
			std::condition_variable _jobs_done;
			std::vector<std::thread> _workers;
			//whether threads are running
			std::atomic<bool> _running;
			//whether threads are actively looking for jobs
			std::atomic<bool> _active;
		};
	}
	struct delay_start_t {};

	/*
		Returns the hardware_concurrency, unless that returns 0, 
		in which case returns def_val.
	*/
	_EXLIB_THREAD_POOL_NODISCARD
	inline decltype(std::thread::hardware_concurrency()) hardware_concurrency_or(decltype(std::thread::hardware_concurrency()) def_val)
	{
		auto const nt=std::thread::hardware_concurrency();
		if(nt)
		{
			return nt;
		}
		return def_val;
	}

	/*
		Returns the hardware_concurrency, unless that returns 0, 
		in which case returns the value lazily determined by the given function.
	*/
	template<typename Func>
	_EXLIB_THREAD_POOL_NODISCARD
	auto hardware_concurrency_or(Func&& f) -> decltype(std::forward<Func>(f)(),std::thread::hardware_concurrency())
	{
		auto const nt=std::thread::hardware_concurrency();
		if(nt)
		{
			return nt;
		}
		return std::forward<Func>(f)();
	}

	/*
		Thread pool where the threadpool stores the given arguments and each task is given those arguments.
		Recommended using thread_pool if using a thread pool multiple times to avoid code bloat.
	*/
	template<typename... Args>
	class thread_pool_a:detail::thread_pool_base {
	public:
		static_assert(detail::no_rvalue_references<Args...>::value,"rvalue references not allowed as arguments");

		friend class parent_ref;
		/*
			A reference to the parent that child tasks can accept. Should be passed by value.
			Contains the methods safe to call by child threads.
		*/
		class parent_ref {
			friend class thread_pool_a;
			thread_pool_a& parent;
			parent_ref(thread_pool_a& p):parent(p)
			{}
		public:
			/*
				Signals threads to stop looking for tasks and will signal a waiting master thread.
			*/
			void stop()
			{
				parent.internal_stop();
			}
			/*
				See thread_pool_a::push_back
			*/
			template<typename... Tasks>
			void push_back(Tasks&&... tasks)
			{
				parent.push_back(std::forward<Tasks>(tasks)...);
			}
			/*
				See thread_pool_a::append
			*/
			template<typename Iter>
			size_t append(Iter begin,Iter end)
			{
				return parent.append(begin,end);
			}
			/*
				Clears tasks.
			*/
			void clear()
			{
				parent.clear();
			}
			/*
				Signals the threads to end. Does not join them.
			*/
			void terminate()
			{
				parent.internal_terminate();
			}
		};
		using thread_pool_base::reactivate;
		using thread_pool_base::active;
		using thread_pool_base::stop;
		using thread_pool_base::running;
		/*
			Starts the threadpool with a certain number of threads and arguments initialized to the given arguments.
		*/
		template<typename... T>
		explicit thread_pool_a(size_t num_threads,T&&... args):thread_pool_base(num_threads,true),_input(std::forward<T>(args)...)
		{
			create_threads();
		}

		/*
			Initializes the threadpool with a certain number of threads and arguments initialized to the given arguments.
			Threads are not started.
		*/
		template<typename... T>
		explicit thread_pool_a(size_t num_threads,delay_start_t,T&&... args):thread_pool_base(num_threads,false),_input(std::forward<T>(args)...)
		{}

		/*
			Starts the threadpool with number of threads equal to the hardware concurrency.
		*/
		thread_pool_a():thread_pool_a(hardware_concurrency_or(1))
		{}

		/*
			Initializes the threadpool with number of threads equal to the hardware concurrency.
			Threads not started.
		*/
		explicit thread_pool_a(delay_start_t t):thread_pool_a(hardware_concurrency_or(1),t)
		{}

		/*
			Set the args passed to the threads. Unsynchronized as you should not be modifying args that are actively being read from.
		*/
		template<typename... TplArgs>
		void set_args(TplArgs&&... args)
		{
			_input=TaskInput(std::forward<TplArgs>(args)...);
		}

		/*
			Starts threads.
		*/
		void start()
		{
			if(!this->_running)
			{
				this->_running=true;
				this->_active=true;
				create_threads();
			}
		}

		/*
			Makes threads look for tasks. Useful after stop() has been called to reactivate job search.
			Resets the args passed to the threads.
		*/
		template<typename... Args>
		void reactivate(Args&&... args)
		{
			set_args(std::forward<Args>(args));
			this->reactivate();
		}

		/*
			Waits for all jobs to be finished or for it to be stop()ed (be inactived).
			Threads keep running (if they already were), but tasks can be added without synchronization.
		*/
		void wait()
		{
			if(this->_active)
			{
				std::unique_lock<std::mutex> lock(this->_mtx);
				this->_jobs_done.wait(lock,[this]
				{
					return this->wait_func();
				});
			}
		}

		/*
			Waits for all jobs to be finished or to be stop()ed.
			If thread pool is not active, does nothing and returns true. Returns false if timeout expired.
		*/
		template<typename Rep,typename Period>
		bool wait_for(std::chrono::duration<Rep,Period> const& rel_time)
		{
			auto absolute_time=std::chrono::steady_clock::now()+rel_time;
			return wait_until(absolute_time);
		}

		/*
			Waits for all jobs to be finished or to be stop()ed.
			If thread pool is not active, does nothing and returns true. Returns false if timeout expired.
		*/
		template<typename Clock,typename Duration>
		bool wait_until(std::chrono::time_point<Clock,Duration> const& rel_time)
		{
			if(this->_active)
			{
				std::unique_lock<std::mutex> lock(this->_mtx);
				return this->_jobs_done.wait_until(lock,rel_time,[this]
				{
					return this->wait_func();
				});
			}
			return true;
		}

		/*
			Makes threads stop looking for jobs and ends threads.
		*/
		void terminate()
		{
			this->stop_running();
			this->join_all();
		}

		/*
			Waits for all jobs to finish and ends threads.
		*/
		void join()
		{
			wait();
			this->_active=false;
			this->_running=false;
			this->_signal_start.notify_all();
			this->join_all();
		}

		/*
			calls join() and then does normal destruction
		*/
		~thread_pool_a()
		{
			join();
		}

		/*
			Adds a task to the thread pool without synchronization.
			Task must define operator() that can take in Args...
			or optionally parent_ref as a first argument and then Args...
		*/
		template<typename Task>
		void push_back_no_sync(Task&& task)
		{
			this->_jobs.push_back(make_job(std::forward<Task>(task)));
		}

		/*
			Adds tasks to the thread pool without synchronization.
			Tasks must define operator() that can take in Args...
			or optionally parent_ref as a first argument and then Args...
		*/
		template<typename FirstTask,typename... Rest>
		void push_back_no_sync(FirstTask&& first,Rest&&... rest)
		{
			push_back_no_sync(std::forward<FirstTask>(first));
			push_back_no_sync(std::forward<Rest>(rest)...);
		}

		/*
			Adds task(s) to the thread pool with synchronization and wakes an appropriate number of threads
			after all tasks have been added.
			Tasks must define operator() that can take in Args...
			or optionally parent_ref as a first argument and then Args...
			Can be called safely by child threads.
		*/
		template<typename... Tasks>
		void push_back(Tasks&&... tasks)
		{
			{
				std::lock_guard<std::mutex> guard(this->_mtx);
				push_back_no_sync(std::forward<Tasks>(tasks)...);
			}
			this->notify_count(sizeof...(Tasks));
		}

		/*
			Adds task(s) to the thread pool without synchronization reading	from the given iterators.
			Tasks must define operator() that can take in Args...
			or optionally parent_ref as a first argument and then Args...
			Returns the number of tasks added.
		*/
		template<typename Iter>
		size_t append_no_sync(Iter begin,Iter end)
		{
			using category=typename std::iterator_traits<Iter>::iterator_category;
			return append_no_sync(begin,end,category{});
		}

		/*
			Adds task(s) to the thread pool from the given iterators
			with synchronization and wakes an appropriate number of threads
			after all tasks have been added.
			Tasks must define operator() that can take in Args...
			or optionally parent_ref as a first argument and then Args...
			Returns the number of tasks added.
			Can be called safely by child threads.
		*/
		template<typename Iter>
		size_t append(Iter begin,Iter end)
		{
			size_t count;
			{
				std::lock_guard<std::mutex> guard(this->_mtx);
				count=append_no_sync(begin,end);
			}
			this->notify_count(count);
			return count;
		}

		/*
			Clears the jobs handled by this threadpool without synchronization.
		*/
		void clear_no_sync()
		{
			this->_jobs.clear();
		}

		/*
			Clears the jobs handled by this threadpool.
		*/
		void clear()
		{
			std::lock_guard<std::mutex> guard(this->_mtx);
			this->clear_no_sync();
		}

	private:
		template<typename Iter>
		size_t append_no_sync(Iter begin,Iter end,std::random_access_iterator_tag)
		{
			std::for_each(begin,end,[this](auto&& task) // std has special hacks to convert some iterators to pointers and prevent code bloat
			{
				this->push_back_no_sync(std::forward<decltype(task)>(task));
			});
			return end-begin;
		}
		template<typename Iter>
		size_t append_no_sync(Iter begin,Iter end,std::input_iterator_tag)
		{
			size_t count=0;
			std::for_each(begin,end,[this,&count](auto&& task)
			{
				this->push_back_no_sync(std::forward<decltype(task)>(task));
				++count;
			});
			return count;
		}
		bool wait_func()
		{
			return !this->_active||this->_jobs.empty();
		}
		void create_threads()
		{
			for(auto& worker:this->_workers)
			{
				worker=std::thread(&thread_pool_a::task_loop,this);
			}
		}
		using TaskInput=std::tuple<detail::wrap_reference_t<Args>...>;
		struct job {
			virtual void operator()(parent_ref,TaskInput const& input)=0;
			virtual ~job()=default;
		};
		template<typename BaseFunc>
		struct job_impl:job {
			BaseFunc task;
			template<typename F>
			job_impl(F&& f):task(std::forward<F>(f))
			{}
			void operator()(parent_ref,TaskInput const& input) override
			{
				detail::apply(task,input);
			}
		};
		template<typename BaseFunc>
		struct job_impl_accept_parent:job {
			BaseFunc task;
			template<typename F>
			job_impl_accept_parent(F&& f):task(std::forward<F>(f))
			{}
			void operator()(parent_ref tp,TaskInput const& input) override
			{
				detail::apply_fa(task,tp,input);
			}
		};

		//overload to try to fit to pass arguments without parent
		template<typename Task,typename... Extra>
		static std::unique_ptr<job> make_job(Task&& the_task,Extra...)
		{
			return make_job2(std::forward<Task>(the_task));
		}

		//overload to try to fit to pass arguments with parent
		template<typename Task>
		static auto make_job(Task&& the_task) -> decltype(std::forward<Task>(the_task)(std::declval<parent_ref>(),std::declval<Args>()...),std::unique_ptr<job>())
		{
			return std::unique_ptr<job>(new job_impl_accept_parent<detail::remove_cvref_t<Task>>(std::forward<Task>(the_task)));
		}

		template<typename Task,typename... Extra>
		static std::unique_ptr<job> make_job2(Task&& the_task,Extra...)
		{
			static_assert(false,"Task fails to accepts proper arguments; must accept (parent_ref, Args...), or (Args...)");
		}

		template<typename Task>
		static auto make_job2(Task&& the_task)  -> decltype(std::forward<Task>(the_task)(std::declval<Args>()...),std::unique_ptr<job>())
		{
			return std::unique_ptr<job>(new job_impl<detail::remove_cvref_t<Task>>(std::forward<Task>(the_task)));
		}

		void task_loop()
		{
			while(true)
			{
				std::unique_ptr<job> task;
				size_t jobs_left;
				{
					if(!this->_running)
					{
						return; //don't bother locking if not running
					}
					std::unique_lock<std::mutex> lock(this->_mtx);
					while(true)
					{
						if(!this->_running)
						{
							return;
						}
						if(this->_active&&(jobs_left=this->_jobs.size()))
						{
							task=std::move(this->_jobs.front());
							this->_jobs.pop_front();
							break;
						}
						this->_signal_start.wait(lock);
					}
				}
				(*task)(parent_ref(*this),this->_input);
				if(jobs_left==1)
				{
					this->_jobs_done.notify_one();
				}
			}
		}
		std::deque<std::unique_ptr<job>> _jobs;
		TaskInput _input;
	};

	using thread_pool=thread_pool_a<>;
}
#endif