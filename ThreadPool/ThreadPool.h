/*
Copyright(C) 2017 Edward Xie

This program is free software:you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation,either version 3 of the License,or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.If not,see <https://www.gnu.org/licenses/>.
*/
#ifndef THREAD_POOL_H
#define THREAD_POOL_H
#ifdef THREADPOOL_EXPORTS
#define THREADPOOL_API __declspec(dllexport)
#else
#define THREADPOOL_API __declspec(dllimport)
#endif
#include <mutex>
#include <queue>
#include <memory>
#include <atomic>
namespace concurrent {
	class ThreadTask {
	private:
	protected:
		ThreadTask()=default;
	public:
		~ThreadTask()=default;
		virtual void execute()=0;
	};

	class ThreadPool {
		friend class std::thread;
	private:
		std::vector<std::thread> workers;
		std::queue<std::unique_ptr<ThreadTask>> tasks;
		std::mutex locker;
		std::atomic<bool> running;
		THREADPOOL_API void do_task();
	public:
		THREADPOOL_API explicit ThreadPool(size_t num_threads);
		THREADPOOL_API ThreadPool();
		THREADPOOL_API ~ThreadPool();
		template<typename T,typename... args>
		void add_task(args&&...);
		template<typename T,typename... args>
		void add_task_sync(args&&...);
		THREADPOOL_API bool is_running() const;
		THREADPOOL_API void start();
		THREADPOOL_API void stop();
	};

	template<typename T,typename... args>
	void ThreadPool::add_task(args&&... arguments) {
		tasks.push(std::make_unique<T>(arguments...));
	}
	template<typename T,typename... args>
	void ThreadPool::add_task_sync(args&&... arguments) {
		std::lock_guard<std::mutex> guard(locker);
		tasks.push(std::make_unique<T>(arguments...));
	}
}
#endif // !THREAD_POOL_H
