#ifndef THREAD_POOL_H
#define THREAD_POOL_H
#include <mutex>
#include <queue>
#include <memory>
#include <atomic>
namespace Concurrent {
	class ThreadPool;
	class ThreadTask;
	class ThreadPool {
	private:
		size_t num_workers;
		std::vector<std::thread> workers;
		std::queue<std::unique_ptr<ThreadTask>> tasks;
		std::mutex locker;
		std::atomic<bool> running;
	public:
		void task();
		explicit ThreadPool(size_t num_threads);
		ThreadPool();
		~ThreadPool();
		template<typename T,typename... args>
		void add_task(args&&...);
		std::unique_ptr<ThreadTask> pop_task();
		bool has_task();
		bool is_running() const;
		void start();
		void stop();
	};
	class ThreadTask {
	private:
	protected:
		ThreadTask()=default;
	public:
		~ThreadTask()=default;
		virtual void execute()=0;
	};

	ThreadPool::ThreadPool(size_t num_threads):num_workers(num_threads),workers(0),running(false),tasks() {
	}

	ThreadPool::ThreadPool():ThreadPool(1U){}

	template<typename T,typename... args>
	void ThreadPool::add_task(args&&... arguments) {
		tasks.push(make_unique<T>(arguments...));
	}
	std::unique_ptr<ThreadTask> ThreadPool::pop_task() {
		std::lock_guard<std::mutex> guard(locker);
		std::unique_ptr<ThreadTask> task=std::move(tasks.front());
		tasks.pop();
		return task;
	}
	bool ThreadPool::is_running() const{
		return running;
	}
	bool ThreadPool::has_task() {
		std::lock_guard<std::mutex> guard(locker);
		return !tasks.empty();
	}
	void ThreadPool::task() {
		std::unique_ptr<ThreadTask> task;
		while(running)
		{
			bool has_task;
			{
				std::lock_guard<std::mutex> guard(locker);
				if(has_task=!tasks.empty())
				{
					task=std::move(tasks.front());
					tasks.pop();
				}
			}
			if(has_task)
			{
				task->execute();
			}
		}
	}
	void ThreadPool::start() {
		running=true;
		for(unsigned int i=0;i<num_workers;++i)
		{
			workers.emplace_back(&ThreadPool::task,this);
		}
	}
	void ThreadPool::stop() {
		running=false;
		for(size_t i=0;i<num_workers;++i)
		{
			workers[i].join();
		}
	}
	ThreadPool::~ThreadPool() {
		stop();
	}
}
#endif // !THREAD_POOL_H
