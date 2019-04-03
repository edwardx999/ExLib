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

#if _EXLIB_THREAD_POOL_HAS_CPP_14
		template<typename Task,typename... Args>
		auto async(Task&& task,Args&&... args)
		{
			return detail::get_pool().async(std::forward<Task>(task),std::forward<Args>(args)...);
		}
#endif
	}
}
#endif