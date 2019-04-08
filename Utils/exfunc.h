/*
Contains unique_function, a move-only version of function

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
#ifndef EXFUNC_H
#define EXFUNC_H
#include <utility>
#include <type_traits>
#include <cstddef>
#include <exception>
namespace exlib {

	class bad_function_call:public std::exception {
	public:
		virtual char const* what() const noexcept override
		{
			return "bad function call";
		}
	};

	namespace unique_func_det {

		template<typename T,bool trivial/*false*/>
		struct deleter {
			constexpr static void do_delete(void* data)
			{
				static_cast<T*>(data)->~T();
			}
		};
		template<typename T>
		struct deleter<T,true> {
			constexpr static void do_delete(void* data)
			{}
		};

#ifdef EX_UNIQUE_FUNCTION_MAX_SIZE
		constexpr size_t max_size=EX_UNIQUE_FUNCTION_MAX_SIZE;
#else
		constexpr size_t max_size=6*sizeof(std::size_t);
#endif

		template<typename T,bool trivial=std::is_trivially_destructible<T>::value,bool fits=sizeof(T)<=max_size>
		struct indirect_deleter {
			constexpr static void do_delete(void* data)
			{
				deleter<T,trivial>::do_delete(data);
			}
		};
		template<typename Func,bool trivial>
		struct indirect_deleter<Func,trivial,false> {
			constexpr static void do_delete(void* data)
			{
				auto const real_data=*static_cast<Func**>(data);
				deleter<Func,trivial>::do_delete(real_data);
				delete real_data;
			}
		};

		template<typename Func>
		struct indirect_deleter<Func,true,true> {
			constexpr static void(*do_delete)(void*)=nullptr;
		};
		
		template<typename Func,typename Sig,bool fits=sizeof(Func)<=max_size>
		struct indirect_call;

		template<typename Func,typename Ret,typename... Args>
		struct indirect_call<Func,Ret(Args...),true> {
			static Ret call_me(void* obj,Args... args)
			{
				return (*static_cast<Func*>(obj))(std::forward<Args>(args)...);
			}
		};
		template<typename Func,typename Ret,typename... Args>
		struct indirect_call<Func,Ret(Args...),false> {
			static Ret call_me(void* obj,Args... args)
			{
				return (**static_cast<Func**>(obj))(std::forward<Args>(args)...);
			}
		};

		template<typename Func,typename Ret,typename... Args>
		struct indirect_call<Func,Ret(Args...) const,true> {
			static Ret call_me(void const* obj,Args... args)
			{
				return (*static_cast<Func const*>(obj))(std::forward<Args>(args)...);
			}
		};

		template<typename Func,typename Ret,typename... Args>
		struct indirect_call<Func,Ret(Args...) const,false> {
			static Ret call_me(void const* obj,Args... args)
			{
				return (**static_cast<Func const* const*>(obj))(std::forward<Args>(args)...);
			}
		};

		template<typename Func,bool fits=sizeof(Func)<=max_size>
		struct func_constructor {
			static void construct(void* location,Func func)
			{
				new (location) Func{std::move(func)};
			}
		};

		template<typename Func>
		struct func_constructor<Func,false> {
			static void construct(void* location,Func func)
			{
				*static_cast<Func**>(location)=new Func{std::move(func)};
			}
		};

		template<typename Derived,typename Sig>
		struct get_call_op;

		template<typename Ret,typename UniqueFunction,typename... Args>
		Ret call_op(UniqueFunction& func,Args&&... args)
		{
			if(func._func)
			{
				return func._func(&func._data,std::forward<Args>(args)...);
			}
			throw exlib::bad_function_call{};
		}

		template<typename Derived,typename Ret,typename... Args>
		struct get_call_op<Derived,Ret(Args...) const> {
			using result_type=Ret;
			Ret operator()(Args... args) const
			{
				return call_op<Ret>(static_cast<Derived const&>(*this),std::forward<Args>(args)...);
			}
		};
		template<typename Derived,typename Ret,typename... Args>
		struct get_call_op<Derived,Ret(Args...)> {
			using result_type=Ret;
			Ret operator()(Args... args)
			{
				return call_op<Ret>(static_cast<Derived&>(*this),std::forward<Args>(args)...);
			}
		};

		template<typename Sig>
		struct get_pfunc;

		template<typename Ret,typename... Args>
		struct get_pfunc<Ret(Args...)> {
			using type=Ret(*)(void*,Args...);
		};
		template<typename Ret,typename... Args>
		struct get_pfunc<Ret(Args...) const> {
			using type=Ret(*)(void const*,Args...);
		};
	}

	template<typename Sig>
	class unique_function:unique_func_det::get_call_op<unique_function<Sig>,Sig> {
		using get_call_op=unique_func_det::get_call_op<unique_function<Sig>,Sig>;
		friend get_call_op;
		template<typename Ret,typename UniqueFunc,typename... Args>
		friend Ret unique_func_det::call_op(UniqueFunc&,Args&&...);

		using DeleterType=void(*)(void*);
		DeleterType _deleter;
		using FuncType=typename unique_func_det::get_pfunc<Sig>::type;
		FuncType _func;
		using Data=typename std::aligned_storage<unique_func_det::max_size,alignof(std::max_align_t)>::type;
		Data _data;
		void cleanup()
		{
			if(_deleter)
			{
				_deleter(&_data);
			}
		}
	public:
		using typename get_call_op::result_type;

		unique_function() noexcept:_deleter{nullptr},_func{nullptr}{}

		template<typename Func>
		unique_function(Func func):
			_deleter{unique_func_det::indirect_deleter<Func>::do_delete},
			_func{unique_func_det::indirect_call<Func,Sig>::call_me}
		{
			static_assert(alignof(Func)<=alignof(Data),"Overaligned functions not supported");
			unique_func_det::func_constructor<Func>::construct(&_data,std::move(func));
		}

		unique_function(unique_function&& o) noexcept:_func{std::exchange(o._func,nullptr)},_deleter{std::exchange(o._deleter,nullptr)},_data{o._data}
		{
		}

		explicit operator bool() const noexcept
		{
			return _func;
		}

		using get_call_op::operator();

		template<typename Func>
		unique_function& operator=(Func func)
		{
			unique_function uf{std::move(func)};
			uf.swap(*this);
			return *this;
		}

		void swap(unique_function& other) noexcept
		{
			std::swap(_deleter,other._deleter);
			std::swap(_func,other._func);
			std::swap(_data,other._data);
		}

		~unique_function()
		{
			cleanup();
		}
	};
}
#endif