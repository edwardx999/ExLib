#pragma once
#ifndef _EXLIB_GLOBAL_THREAD_POOL_H
#define _EXLIB_GLOBAL_THREAD_POOL_H
#include "thread_pool.h"
#include <future>
namespace exlib {
	namespace global_thread_pool {
		namespace detail {
			thread_pool& get_pool();
		}
		template<typename Iter>
		void append(Iter begin,Iter end) {
			detail::get_pool().append(begin,end);
		}
		template<typename Iter>
		void prepend(Iter begin,Iter end) {
			detail::get_pool().prepend(begin,end);
		}
		template<typename... Tasks>
		void push_back(Tasks&&... tasks)
		{
			detail::get_pool().push_back(std::forward<Tasks>(tasks)...);
		}
		template<typename... Tasks>
		void push_front(Tasks&&... tasks)
		{
			detail::get_pool().push_front(std::forward<Tasks>(tasks)...);
		}

		namespace detail {
			template<typename ReturnType>
			struct set_promise_value {
				template<typename Task,typename... Args>
				static void set(std::promise<ReturnType>& promise,Task&& task,Args&&... args)
				{
					promise.set_value(std::forward<Task>(task)(std::forward<Args>(args)...));
				}
			};
			template<>
			struct set_promise_value<void> {
				template<typename Task, typename... Args>
				static void set(std::promise<void>& promise, Task&& task, Args&& ... args)
				{
					std::forward<Task>(task)(std::forward<Args>(args)...);
					promise.set_value();
				}
			};
		}

		template<typename Task,typename... Args>
		_EXLIB_THREAD_POOL_NODISCARD auto async(Task&& task,Args&&... args) -> std::future<decltype(std::forward<Task>(task)(std::forward<Args>(args)...))>
		{	
			using Type=decltype(std::forward<Task>(task)(std::forward<Args>(args)...));
			std::promise<Type> promise;
			auto future=promise.get_future();
			detail::get_pool().push_back([promise=std::move(promise),task=std::forward<Task>(task),args=std::forward<Args>(args)...](thread_pool::parent_ref) mutable
			{
				try 
				{
					detail::set_promise_value<Type>::set(promise,std::forward<Task>(task),std::forward<Args>(args)...);
				}
				catch(...)
				{
					promise.set_exception(std::current_exception());
				}
			});
			return future;
		}
	}
}
#endif