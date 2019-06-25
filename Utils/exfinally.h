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
#ifndef EXFINALLY_H
#define EXFINALLY_H
#ifdef _MSVC_LANG
#define EXFINALLY_HAS_CPP17 (_MSVC_LANG>=201700l)
#else
#define EXFINALLY_HAS_CPP17 (__cplusplus>=201700l)
#endif
#if EXFINALLY_HAS_CPP17
#define EXFINALLY_NODISCARD [[nodiscard]]
#else
#define EXFINALLY_NODISCARD
#endif
#include "exretype.h"
#include <type_traits>
#include <cstring>
namespace exlib {

	/*
		Invokes the given functor when it goes out of scope.
	*/
	template<typename Finally>
	struct EXFINALLY_NODISCARD finally:exlib::empty_store<Finally> {
		using Base=exlib::empty_store<Finally>;
	public:
		template<typename F>
		finally(F&& f): Base{std::forward<F>(f)}
		{
		}
		finally(finally const&) = delete;
		finally& operator=(finally const&) = delete;
		~finally()
		{
			this->get()();
		}
	};

#ifdef __cpp_deduction_guides
	template<typename F>
	finally(F&& f)->finally<std::decay_t<F>>;
#endif
}
#endif