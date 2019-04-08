/*
Copyright 2019 Edward Xie

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
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

		template<typename Task>
		_EXLIB_THREAD_POOL_NODISCARD auto async(Task&& task) -> decltype(detail::get_pool().async(std::forward<Task>(task)))
		{
			return detail::get_pool().async(std::forward<Task>(task));
		}
	}
}
#endif