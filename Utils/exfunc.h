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
#include "exretype.h"
#ifdef _MSVC_LANG
#define _EXFUNC_HAS_CPP_17 (_MSVC_LANG>=201700L)
#else
#define _EXFUNC_HAS_CPP_17 (__cplusplus>=201700L)
#endif
#ifndef EX_UNIQUE_FUNCTION_USE_CPP_17_OPTIMIZATIONS
#define EX_UNIQUE_FUNCTION_USE_CPP_17_OPTIMIZATIONS _EXFUNC_HAS_CPP_17
#endif
namespace exlib {

	class bad_function_call:public std::exception {
	public:
		char const* what() const noexcept override
		{
			return "bad function call";
		}
	};

	struct nothrow_destructor_tag {};

	namespace unique_func_det {

		template<typename... Signatures>
		struct func_pack {
			static constexpr auto size=sizeof...(Signatures);
		};

		template<size_t I,typename FuncPack>
		struct func_element;

		template<typename First,typename... Signatures>
		struct func_element<0,func_pack<First,Signatures...>> {
			using type=First;
		};

		template<size_t I,typename First,typename... Signatures>
		struct func_element<I,func_pack<First,Signatures...>>:func_element<I-1,func_pack<Signatures...>> {
		};

		template<typename First,typename TupleRest>
		struct cons_pack;

		template<typename First,typename... Types>
		struct cons_pack<First,func_pack<Types...>> {
			using type=func_pack<First,Types...>;
		};

#ifdef EX_UNIQUE_FUNCTION_MAX_SIZE
		constexpr std::size_t max_size=EX_UNIQUE_FUNCTION_MAX_SIZE;
#else
		constexpr std::size_t max_size=6*sizeof(std::size_t);
#endif
		static_assert(max_size>=sizeof(void*),"Unique function must be able to fit at least a pointer.");
		constexpr std::size_t alignment=alignof(std::max_align_t);

		template<typename T>
		struct type_fits:std::integral_constant<bool,(alignof(T)<=alignment)&&(sizeof(T)<=max_size)&&std::is_nothrow_move_constructible<T>::value&&std::is_nothrow_move_assignable<T>::value>{};

		template<typename T,bool trivial=std::is_trivially_destructible<T>::value,bool fits=type_fits<T>::value>
		struct indirect_deleter {
			static void do_delete(void* data) noexcept
			{
				static_cast<T*>(data)->~T();
			}
		};
		template<typename Func,bool trivial>
		struct indirect_deleter<Func,trivial,false> {
			constexpr static void do_delete(void* data) noexcept
			{
				auto const real_data=*static_cast<Func**>(data);
				delete real_data;
			}
		};

		struct trivial_deleter {
			static constexpr void (*do_delete)(void*)=nullptr;
		};

		using DeleterFunc=typename std::remove_const<decltype(trivial_deleter::do_delete)>::type;

		template<typename Func>
		struct indirect_deleter<Func,true,true>:trivial_deleter {
		};
		
		template<typename Func,typename Sig,bool fits=type_fits<Func>::value>
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

#if _EXFUNC_HAS_CPP_17
		template<typename Func,typename Ret,typename... Args>
		struct indirect_call<Func,Ret(Args...) noexcept,true> {
			static Ret call_me(void* obj,Args... args) noexcept
			{
				return (*static_cast<Func*>(obj))(std::forward<Args>(args)...);
			}
		};
		template<typename Func,typename Ret,typename... Args>
		struct indirect_call<Func,Ret(Args...) noexcept,false> {
			static Ret call_me(void* obj,Args... args) noexcept
			{
				return (**static_cast<Func**>(obj))(std::forward<Args>(args)...);
			}
		};

		template<typename Func,typename Ret,typename... Args>
		struct indirect_call<Func,Ret(Args...) const noexcept,true> {
			static Ret call_me(void const* obj,Args... args) noexcept
			{
				return (*static_cast<Func const*>(obj))(std::forward<Args>(args)...);
			}
		};
		template<typename Func,typename Ret,typename... Args>
		struct indirect_call<Func,Ret(Args...) const noexcept,false> {
			static Ret call_me(void const* obj,Args... args) noexcept
			{
				return (**static_cast<Func const* const*>(obj))(std::forward<Args>(args)...);
			}
		};
#endif

#pragma warning(push)
#pragma warning(disable:4789) //MSVC complains about initializing overaligned types even when the other specialization is used
		template<typename Func,bool fits=type_fits<Func>::value>
		struct func_constructor {
			static void construct(void* location,Func func)
			{
				new (location) Func{std::move(func)};
			}
		};
#pragma warning(pop)

		template<typename Func>
		struct func_constructor<Func,false> {
			static void construct(void* location,Func func)
			{
#if !_EXFUNC_HAS_CPP_17
				static_assert(alignof(Func)<=alignment,"Overaligned functions not supported");
#endif
				*static_cast<Func**>(location)=new Func{std::move(func)};
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

		template<typename... Types>
		struct has_nothrow_tag;

		template<>
		struct has_nothrow_tag<>:std::false_type {};

		template<typename First,typename... Rest>
		struct has_nothrow_tag<First,Rest...>:has_nothrow_tag<Rest...> {};

		template<typename... Rest>
		struct has_nothrow_tag<nothrow_destructor_tag,Rest...>:std::true_type {};

		template<typename SigTuple,bool in_place=(
#if EX_UNIQUE_FUNCTION_USE_CPP_17_OPTIMIZATIONS
			SigTuple::size==1
#else
			true
#endif
			)>
		struct func_table_from_tup;

		template<typename Func,typename SigTuple,typename IndexSeq=make_index_sequence<SigTuple::size>>
		struct func_table_holder_help;

		void placeholder_function();
		using vpfunc=decltype(&placeholder_function);

		template<typename Func,typename SigTuple,size_t... Is>
		struct func_table_holder_help<Func,SigTuple,index_sequence<Is...>>{
			constexpr static vpfunc const func_table[SigTuple::size?SigTuple::size:1]={reinterpret_cast<vpfunc>(&indirect_call<Func,typename func_element<Is,SigTuple>::type>::call_me)...};
		};

		template<typename Func,typename SigTuple>
		struct func_table_holder:func_table_holder_help<Func,SigTuple> {
		};

		template<typename SigTuple>
		struct func_table_from_tup<SigTuple,false> {
		private:
			vpfunc const* _func_table;
		public:
			func_table_from_tup():_func_table{}{}
			template<typename Func>
			func_table_from_tup(Func const&):_func_table{func_table_holder<Func,SigTuple>::func_table}
			{}

			vpfunc const* get_table() const noexcept
			{
				return _func_table;
			}

			explicit operator bool() const noexcept
			{
				return _func_table;
			}

			void swap(func_table_from_tup& o) noexcept
			{
				std::swap(_func_table,o._func_table);
			}
		};

		template<typename SigTuple,typename IndexSeq=make_index_sequence<SigTuple::size>>
		struct func_table_from_tup_inplace_help;

		template<typename SigTuple,size_t... Is>
		struct func_table_from_tup_inplace_help<SigTuple,index_sequence<Is...>>
		{
		private:
			vpfunc _func_table[SigTuple::size?SigTuple::size:1];
		public:
			func_table_from_tup_inplace_help():_func_table{}{}

			template<typename Func>
			func_table_from_tup_inplace_help(Func const&):_func_table{reinterpret_cast<vpfunc>(&indirect_call<Func,typename func_element<Is,SigTuple>::type>::call_me)...}
			{}

			vpfunc const* get_table() const noexcept
			{
				return _func_table;
			}

			explicit operator bool() const noexcept
			{
				if
#if _EXFUNC_HAS_CPP_17
					constexpr
#endif
					(SigTuple::size == 0)
				{
					return false;
				}
				return _func_table[0];
			}

			void swap(func_table_from_tup_inplace_help& o) noexcept
			{
				std::swap(_func_table,o._func_table);
			}
		};

		template<typename SigTuple>
		struct func_table_from_tup<SigTuple,true>:func_table_from_tup_inplace_help<SigTuple> {
			using func_table_from_tup_inplace_help<SigTuple>::func_table_from_tup_inplace_help;
		};

		template<typename... Signatures>
		struct strip_nothrow_tags;

		template<>
		struct strip_nothrow_tags<> {
			using type=func_pack<>;
		};

		template<typename... Rest>
		struct strip_nothrow_tags<nothrow_destructor_tag,Rest...> {
			using type=typename strip_nothrow_tags<Rest...>::type;
		};

		template<typename First,typename... Rest>
		struct strip_nothrow_tags<First,Rest...>:cons_pack<First,typename strip_nothrow_tags<Rest...>::type> {
		};

		template<typename... Signatures>
		struct func_table:func_table_from_tup<typename strip_nothrow_tags<Signatures...>::type>{
			using func_table_from_tup<typename strip_nothrow_tags<Signatures...>::type>::func_table_from_tup;
		};

		template<size_t I,typename Derived,typename Sig>
		struct get_call_op;

		template<size_t I,typename PVoid,typename Ret,typename UniqueFunction,typename... Args>
		Ret call_op(UniqueFunction& func,Args&&... args)
		{
			if(func)
			{
				return (*reinterpret_cast<Ret(*)(PVoid,Args...)>(func.get_table()[I]))(&func._data,std::forward<Args>(args)...);
			}
			throw exlib::bad_function_call{};
		}

		template<size_t I,typename Derived,typename Ret,typename... Args>
		struct get_call_op<I,Derived,Ret(Args...) const> {
			Ret operator()(Args... args) const
			{
				return call_op<I,void const*,Ret>(static_cast<Derived const&>(*this),std::forward<Args>(args)...);
			}
		};
		template<size_t I,typename Derived,typename Ret,typename... Args>
		struct get_call_op<I,Derived,Ret(Args...)> {
			Ret operator()(Args... args)
			{
				return call_op<I,void*,Ret>(static_cast<Derived&>(*this),std::forward<Args>(args)...);
			}
		};

#if _EXFUNC_HAS_CPP_17
		template<size_t I,typename Derived,typename Ret,typename... Args>
		struct get_call_op<I,Derived,Ret(Args...) const noexcept> {
			using result_type=Ret;
			Ret operator()(Args... args) const noexcept
			{
				return call_op<I,void const*,Ret>(static_cast<Derived const&>(*this),std::forward<Args>(args)...);
			}
		};
		template<size_t I,typename Derived,typename Ret,typename... Args>
		struct get_call_op<I,Derived,Ret(Args...) noexcept> {
			using result_type=Ret;
			Ret operator()(Args... args) noexcept
			{
				return call_op<I,void*,Ret>(static_cast<Derived&>(*this),std::forward<Args>(args)...);
			}
		};
#endif

		template<typename Derived,typename SigTuple>
		struct get_call_op_table;

		template<size_t I,typename Derived,typename... Sigs>
		struct get_call_op_table_help;

		template<size_t I,typename Derived,typename Sig>
		struct get_call_op_table_help<I,Derived,Sig>:get_call_op<I,Derived,Sig> {
			using get_call_op<I,Derived,Sig>::operator();
		};

		template<size_t I,typename Derived,typename Sig,typename... Rest>
		struct get_call_op_table_help<I,Derived,Sig,Rest...>:get_call_op<I,Derived,Sig>,get_call_op_table_help<I+1,Derived,Rest...> {
			using get_call_op<I,Derived,Sig>::operator();
			using get_call_op_table_help<I+1,Derived,Rest...>::operator();
		};

		template<size_t I,typename Derived>
		struct get_call_op_table_help<I,Derived> {
		};

		template<typename Derived,typename... Sigs>
		struct get_call_op_table<Derived,func_pack<Sigs...>>:get_call_op_table_help<0,Derived,Sigs...> {
		};

		template<typename SigTuple>
		struct get_result_type{};

		template<typename Ret,typename... Args>
		struct get_result_type<func_pack<Ret(Args...)>> {
			using result_type=Ret;
		};

		template<typename Ret,typename... Args>
		struct get_result_type<func_pack<Ret(Args...) const>> {
			using result_type=Ret;
		};

#if _EXFUNC_HAS_CPP_17
		template<typename Ret,typename... Args>
		struct get_result_type<func_pack<Ret(Args...) noexcept>> {
			using result_type=Ret;
		};

		template<typename Ret,typename... Args>
		struct get_result_type<func_pack<Ret(Args...) const noexcept>> {
			using result_type=Ret;
		};
#endif
	}
	/*
		Template arguments are function signatures that may be additionally const and noexcept (C++17+) qualified.
		If nothrow_destructor_tag is found anywhere in the argument list, the desctructor is non throwing.
		Small object optimization enabled for types that are nothrow move constructible/assignable and less than
		unique_func_det::max_size, which can be customized with EX_UNIQUE_FUNCTION_MAX_SIZE (breaks ABI compatibility).
	*/
	template<typename... Signatures>
	class unique_function:
		unique_func_det::func_table<Signatures...>,
		public unique_func_det::get_call_op_table<unique_function<Signatures...>,typename unique_func_det::strip_nothrow_tags<Signatures...>::type>,
		public unique_func_det::get_result_type<typename unique_func_det::strip_nothrow_tags<Signatures...>::type> {

		using get_call_op=unique_func_det::get_call_op_table<unique_function<Signatures...>,typename unique_func_det::strip_nothrow_tags<Signatures...>::type>;
		friend get_call_op;

		template<size_t I,typename Derived,typename Sig>
		friend struct unique_func_det::get_call_op;

		using func_table=unique_func_det::func_table<Signatures...>;

		template<size_t I,typename PVoid,typename Ret,typename UniqueFunc,typename... Args>
		friend Ret unique_func_det::call_op(UniqueFunc&,Args&&...);

		unique_func_det::DeleterFunc _deleter;

		using Data=typename std::aligned_storage<unique_func_det::max_size,unique_func_det::alignment>::type;
		Data _data;

		void cleanup()
		{
			if(_deleter)
			{
				_deleter(&_data);
			}
		}
	public:

		unique_function() noexcept:_deleter{nullptr},func_table{}{}

		template<typename Func>
		unique_function(Func func):
			_deleter{unique_func_det::indirect_deleter<Func>::do_delete},
			func_table{func}
		{
			unique_func_det::func_constructor<Func>::construct(&_data,std::move(func));
		}

		unique_function(unique_function&& o) noexcept:func_table{static_cast<func_table&>(o)},_deleter{o._deleter},_data{o._data}
		{
			static_cast<func_table&>(o)=func_table{};
			o._deleter=nullptr;
		}

		using func_table::operator bool;

		template<typename Func>
		unique_function& operator=(Func func)
		{
			unique_function{std::move(func)}.swap(*this);
			return *this;
		}

		unique_function& operator=(unique_function&& func) noexcept
		{
			unique_function{std::move(func)}.swap(*this);
			return *this;
		}

		void swap(unique_function& other) noexcept
		{
			std::swap(_deleter,other._deleter);
			static_cast<func_table&>(*this).swap(static_cast<func_table&>(other));
			std::swap(_data,other._data);
		}

		~unique_function() noexcept(unique_func_det::has_nothrow_tag<Signatures...>::value)
		{
			cleanup();
		}
	};
}
#endif