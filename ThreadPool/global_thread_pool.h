#pragma once
#ifndef _EXLIB_GLOBAL_THREAD_POOL_H
#define _EXLIB_GLOBAL_THREAD_POOL_H
#include "thread_pool.h"
namespace exlib {
	namespace global_thread_pool {
		namespace detail {
			extern thread_pool pool;
		}
		template<typename Iter>
		void append(Iter begin, Iter end) {
			pool.append(begin, end);
		}
		template<typename Iter>
		void prepend(Iter begin, Iter end) {
			pool.prepend(begin, end);
		}
		template<typename... Tasks>
		void push_back(Tasks&& ... tasks)
		{
			pool.push_back(std::forward<Tasks>(tasks)...);
		}
		template<typename... Tasks>
		void push_front(Tasks&& ... tasks)
		{
			pool.push_front(std::forward<Tasks>(tasks)...);
		}
	}
}
#endif