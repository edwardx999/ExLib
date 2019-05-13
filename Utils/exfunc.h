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
#if	_EXFUNC_HAS_CPP_17
#define _EXFUNC_CONSTEXPRIF constexpr
#ifdef __cpp_lib_hardware_interference_size
#define _EXFUNC_HAS_HARDWARE_INTERFERENCE_SIZE 1
#include <new>
#endif
#else
#define _EXFUNC_HAS_HARDWARE_INTERFERENCE_SIZE 0
#define _EXFUNC_CONSTEXPRIF
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

#ifndef EX_UNIQUE_FUNCTION_INPLACE_TABLE_COUNT
#define EX_UNIQUE_FUNCTION_INPLACE_TABLE_COUNT 1
#endif

#ifdef EX_UNIQUE_FUNCTION_MAX_SIZE
		constexpr std::size_t max_size() noexcept
		{
			return EX_UNIQUE_FUNCTION_MAX_SIZE;
		}
#else
		constexpr std::size_t max(std::ptrdiff_t a,std::ptrdiff_t b)
		{
			return a<b?b:a;
		}
		constexpr std::size_t max_size() noexcept
		{
			return max(
#if _EXFUNC_HAS_HARDWARE_INTERFERENCE_SIZE
				std::hardware_constructive_interference_size
#else 
				64
#endif
			,sizeof(void*));
		}
#define EX_UNIQUE_FUNCTION_MAX_SIZE ::exlib::unique_func_det::max_size()
#endif

		constexpr std::size_t alignment() noexcept
		{
			return alignof(std::max_align_t);
		}

		template<typename T>
		struct type_fits:std::integral_constant<bool,(alignof(T)<=alignment())&&(sizeof(T)<=max_size())&&std::is_nothrow_move_constructible<T>::value&&std::is_nothrow_move_assignable<T>::value>{};

		using DeleterFunc=void(*)(void*);

		template<typename T,bool trivial=std::is_trivially_destructible<T>::value,bool fits=type_fits<T>::value>
		struct indirect_deleter {
			static void do_delete(void* data) noexcept
			{
				static_cast<T*>(data)->~T();
			}
			static constexpr DeleterFunc get_deleter() noexcept
			{
				return &do_delete;
			}
		};
		template<typename Func,bool trivial>
		struct indirect_deleter<Func,trivial,false> {
			constexpr static void do_delete(void* data) noexcept
			{
				auto const real_data=*static_cast<Func**>(data);
				delete real_data;
			}
			static constexpr DeleterFunc get_deleter() noexcept
			{
				return &do_delete;
			}
		};

		struct trivial_deleter {
			static constexpr DeleterFunc get_deleter() noexcept
			{
				return nullptr;
			}
		};

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
			template<typename U,typename... Args>
			static void construct(void* location,std::initializer_list<U> il,Args&&... args)
			{
				new (location) Func(il,std::forward<Args>(args)...);
			}
			template<typename... Args>
			static void construct(void* location,Args&&... args)
			{
				new (location) Func(std::forward<Args>(args)...);
			}
		};
#pragma warning(pop)

		template<typename Func>
		struct func_constructor<Func,false> {
#if !_EXFUNC_HAS_CPP_17
				static_assert(alignof(Func)<=alignment(),"Overaligned functions not supported");
#endif
			template<typename U,typename... Args>
			static void construct(void* location,std::initializer_list<U> il,Args&&... args)
			{
				*static_cast<Func**>(location)=new Func(il,std::forward<Args>(args)...);
			}
			template<typename... Args>
			static void construct(void* location,Args&&... args)
			{
				*static_cast<Func**>(location)=new Func(std::forward<Args>(args)...);
			}
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
			SigTuple::size<=EX_UNIQUE_FUNCTION_INPLACE_TABLE_COUNT
			)>
		struct func_table_from_tup;

		template<typename Func,typename SigTuple,typename IndexSeq=make_index_sequence<SigTuple::size>>
		struct func_table_holder_help;

		void placeholder_function();
		using vpfunc=decltype(&placeholder_function);

#define _EXFUNC_FUNC_TABLE_DEFINITION(Func) {reinterpret_cast<vpfunc>(indirect_deleter<Func>::get_deleter()),reinterpret_cast<vpfunc>(&indirect_call<Func,typename func_element<Is,SigTuple>::type>::call_me)...}
		template<typename Func,typename SigTuple,size_t... Is>
		struct func_table_holder_help<Func,SigTuple,index_sequence<Is...>>{
#if !_EXFUNC_HAS_CPP_17
			static vpfunc const* get_func_table() noexcept
			{
				static vpfunc const func_table[]=_EXFUNC_FUNC_TABLE_DEFINITION(Func);
				return func_table;
			}
#else
			inline static vpfunc const func_table[]=_EXFUNC_FUNC_TABLE_DEFINITION(Func);
			constexpr static auto get_func_table() noexcept
			{
				return func_table;
			}
#endif
		};

		template<typename Func,typename SigTuple>
		struct func_table_holder:func_table_holder_help<Func,SigTuple> {
		};

		template<typename SigTuple>
		struct func_table_from_tup<SigTuple,false> {
		private:
			vpfunc const* _func_table;
		public:
			constexpr static bool is_inplace=false;
			func_table_from_tup():_func_table{}{}
			template<typename FuncHolder>
			func_table_from_tup(FuncHolder) noexcept:_func_table{func_table_holder<typename FuncHolder::type,SigTuple>::get_func_table()}
			{}

			vpfunc const* get_table() const noexcept
			{
				return _func_table;
			}

			//whether this holds a function
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
			vpfunc _func_table[SigTuple::size+1];
		public:
			func_table_from_tup_inplace_help():_func_table{}{}

			template<typename FuncHolder>
			func_table_from_tup_inplace_help(FuncHolder) noexcept: _func_table _EXFUNC_FUNC_TABLE_DEFINITION(typename FuncHolder::type)
			{}

			vpfunc const* get_table() const noexcept
			{
				return _func_table;
			}

			//whether this holds a function
			explicit operator bool() const noexcept
			{
				if _EXFUNC_CONSTEXPRIF(SigTuple::size<1)
				{
					return false;
				}
				return _func_table[1];
			}

			void swap(func_table_from_tup_inplace_help& o) noexcept
			{
				std::swap(_func_table,o._func_table);
			}
		};

#undef _EXFUNC_FUNC_TABLE_DEFINITION

		template<typename SigTuple>
		struct func_table_from_tup<SigTuple,true>:func_table_from_tup_inplace_help<SigTuple> {
			constexpr static bool is_inplace=true;
			using func_table_from_tup_inplace_help<SigTuple>::func_table_from_tup_inplace_help;
		};

		template<typename... Signatures>
		struct strip_nothrow_tags;

		template<>
		struct strip_nothrow_tags<> {
			using type=func_pack<>;
		};

		template<typename... Rest>
		struct strip_nothrow_tags<nothrow_destructor_tag,Rest...>:strip_nothrow_tags<Rest...> {
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

		template<size_t I,typename PVoid,typename Ret,bool nothrow,typename UniqueFunction,typename... Args>
		Ret call_op(UniqueFunction& func,Args&&... args)
		{
			if(nothrow||func)
			{
				return (*reinterpret_cast<Ret(*)(PVoid,Args...)>(func.get_table()[I]))(&func._data,std::forward<Args>(args)...);
			}
			throw exlib::bad_function_call{};
		}

		template<size_t I,typename Derived,typename Ret,typename... Args>
		struct get_call_op<I,Derived,Ret(Args...) const> {
			//Calls held function, throws bad_function_call if no function held
			Ret operator()(Args... args) const
			{
				return call_op<I,void const*,Ret,false>(static_cast<Derived const&>(*this),std::forward<Args>(args)...);
			}
		};
		template<size_t I,typename Derived,typename Ret,typename... Args>
		struct get_call_op<I,Derived,Ret(Args...)> {
			//Calls held function, throws bad_function_call if no function held
			Ret operator()(Args... args)
			{
				return call_op<I,void*,Ret,false>(static_cast<Derived&>(*this),std::forward<Args>(args)...);
			}
		};

#if _EXFUNC_HAS_CPP_17
		template<size_t I,typename Derived,typename Ret,typename... Args>
		struct get_call_op<I,Derived,Ret(Args...) const noexcept> {
			using result_type=Ret;
			//Calls held function, undefined behavior if no function held
			Ret operator()(Args... args) const noexcept
			{
				return call_op<I,void const*,Ret,true>(static_cast<Derived const&>(*this),std::forward<Args>(args)...);
			}
		};
		template<size_t I,typename Derived,typename Ret,typename... Args>
		struct get_call_op<I,Derived,Ret(Args...) noexcept> {
			using result_type=Ret;
			//Calls held function, undefined behavior if no function held
			Ret operator()(Args... args) noexcept
			{
				return call_op<I,void*,Ret,true>(static_cast<Derived&>(*this),std::forward<Args>(args)...);
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
		struct get_call_op_table<Derived,func_pack<Sigs...>>:get_call_op_table_help<1,Derived,Sigs...> {
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

	template<typename T>
	struct in_place_type {
		explicit in_place_type() noexcept=default;
		using type=T;
	};

	/*
		Template arguments are function signatures that may be additionally const and noexcept (C++17+) qualified.
		If nothrow_destructor_tag is found anywhere in the argument list, the desctructor is non throwing.
		Small object optimization enabled for types that are nothrow move constructible/assignable and will
		fit in this object (total size - vtable space), which can be customized with EX_UNIQUE_FUNCTION_MAX_SIZE (breaks ABI compatibility).
		The vtable may be stored in place depending on the number of signatures as given by EX_UNIQUE_FUNCTION_INPLACE_TABLE_COUNT (default 1).
	*/
	template<typename... Signatures>
	class unique_function:
		unique_func_det::func_table<Signatures...>,
		unique_func_det::get_call_op_table<unique_function<Signatures...>,typename unique_func_det::strip_nothrow_tags<Signatures...>::type>,
		public unique_func_det::get_result_type<typename unique_func_det::strip_nothrow_tags<Signatures...>::type> {

		using get_call_op=unique_func_det::get_call_op_table<unique_function<Signatures...>,typename unique_func_det::strip_nothrow_tags<Signatures...>::type>;
		friend get_call_op;

		template<size_t I,typename Derived,typename Sig>
		friend struct unique_func_det::get_call_op;

		using func_table=unique_func_det::func_table<Signatures...>;

		template<size_t I,typename PVoid,typename Ret,bool nothrow,typename UniqueFunc,typename... Args>
		friend Ret unique_func_det::call_op(UniqueFunc&,Args&&...);

		using Data=typename std::aligned_storage<unique_func_det::max(unique_func_det::max_size()-sizeof(func_table),sizeof(void*)),unique_func_det::alignment()>::type;
		Data _data;

		void cleanup()
		{
			if _EXFUNC_CONSTEXPRIF(!func_table::is_inplace)
			{
				if(!this->get_table())
				{
					return;
				}
			}
			auto deleter=reinterpret_cast<void(*)(void*)>(this->get_table()[0]);
			if(deleter)
			{
				deleter(&_data);
			}
		}

		static constexpr bool noexcept_destructor=unique_func_det::has_nothrow_tag<Signatures...>::value;
	public:

		//default constructor, no held function
		unique_function() noexcept:func_table{}{}

		//nullptr constructor, no held function
		unique_function(std::nullptr_t) noexcept:unique_function{}{}

		//copies or moves a function from an existing function-like object
		template<typename Func>
		unique_function(Func&& func):func_table{std::decay<Func>{}}
		{
			unique_func_det::func_constructor<typename std::decay<Func>::type>::construct(&_data,std::forward<Func>(func));
		}

		//constructs the function given by in_place_type in place using the given arguments
		template<typename Func,typename... Args>
		unique_function(in_place_type<Func> a,Args&&... args):func_table{a}
		{
			unique_func_det::func_constructor<Func>::construct(&_data,std::forward<Args>(args)...);
		}

		//constructs the function given by in_place_type in place using the given arguments
		template<typename Func,typename U,typename... Args>
		unique_function(in_place_type<Func> a,std::initializer_list<U> il,Args&&... args):func_table{a}
		{
			unique_func_det::func_constructor<Func>::construct(&_data,il,std::forward<Args>(args)...);
		}

		//constructs the function given by in_place_type in place using the given arguments
		template<typename Func,typename... Args>
		unique_function& emplace(Args&&... args) &
		{
			unique_function(in_place_type<Func>{},std::forward<Args>(args)...).swap(*this);
			return *this;
		}

		//constructs the function given by in_place_type in place using the given arguments
		template<typename Func,typename U,typename... Args>
		unique_function& emplace(std::initializer_list<U> il,Args&&... args) &
		{
			unique_function(in_place_type<Func>{},il,std::forward<Args>(args)...).swap(*this);
			return *this;
		}

		//constructs the function given by in_place_type in place using the given arguments
		template<typename Func,typename... Args>
		unique_function&& emplace(Args&&... args) &&
		{
			return std::move(emplace<Func>(std::forward<Args>(args)...));
		}

		//constructs the function given by in_place_type in place using the given arguments
		template<typename Func,typename U,typename... Args>
		unique_function&& emplace(std::initializer_list<U> il,Args&&... args) &&
		{
			return std::move(emplace<Func>(il,std::forward<Args>(args)...));
		}

		unique_function(unique_function&& o) noexcept(noexcept_destructor):func_table{static_cast<func_table&>(o)},_data{o._data}
		{
			static_cast<func_table&>(o)=func_table{};
		}

		using func_table::operator bool;

		template<typename Func>
		unique_function& operator=(Func func)
		{
			unique_function{std::move(func)}.swap(*this);
			return *this;
		}

		unique_function& operator=(unique_function&& func) noexcept(noexcept_destructor)
		{
			unique_function{std::move(func)}.swap(*this);
			return *this;
		}

		void swap(unique_function& other) noexcept
		{
			static_cast<func_table&>(*this).swap(static_cast<func_table&>(other));
			std::swap(_data,other._data);
		}

		void reset() noexcept(noexcept_destructor)
		{
			unique_function{}.swap(*this);
		}

		~unique_function() noexcept(noexcept_destructor)
		{
			cleanup();
		}

		using get_call_op::operator();
	};

	template<typename... Signatures>
	void swap(unique_function<Signatures...>& a,unique_function<Signatures...>& b) noexcept
	{
		a.swap(b);
	}

	template<typename... Signatures>
	bool operator==(std::nullptr_t,unique_function<Signatures...> const& f) noexcept
	{
		return !f;
	}
	template<typename... Signatures>
	bool operator!=(std::nullptr_t,unique_function<Signatures...> const& f) noexcept
	{
		return bool{f};
	}
	template<typename... Signatures>
	bool operator==(unique_function<Signatures...> const& f,std::nullptr_t) noexcept
	{
		return !f;
	}
	template<typename... Signatures>
	bool operator!=(unique_function<Signatures...> const& f,std::nullptr_t) noexcept
	{
		return bool{f};
	}
}
#endif