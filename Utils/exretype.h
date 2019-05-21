#pragma once
/*
Contains utilities for combining and recreating types.

Copyright 2018 Edward Xie

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#ifndef EXRETYPE_H
#define EXRETYPE_H
#ifdef _MSVC_LANG
#define _EXRETYPE_HAS_CPP_20 (_MSVC_LANG>=202000L)
#define _EXRETYPE_HAS_CPP_17 (_MSVC_LANG>=201700L)
#define _EXRETYPE_HAS_CPP_14 (_MSVC_LANG>=201400L)
#else
#define _EXRETYPE_HAS_CPP_20 (__cplusplus>=202000L)
#define _EXRETYPE_HAS_CPP_17 (__cplusplus>=201700L)
#define _EXRETYPE_HAS_CPP_14 (__cplusplus>=201400L)
#endif
#if _EXRETYPE_HAS_CPP_14
#define _EX_CONSTEXPR_OVERLOAD constexpr
#else
#define _EX_CONSTEXPR_OVERLOAD
#endif
#include <type_traits>
#include <cstdint>
namespace std {
	template<typename T,std::size_t N>
	class array;
#if _EXRETYPE_HAS_CPP_20
	template<typename T,std::size_t N>
	class span;
#endif
}
namespace exlib {

	template<typename... Bases>
	struct combined:Bases...{

	};

	namespace empty_store_impl {
		template<typename Type,bool ShouldInherit=std::is_empty<Type>::value&& 
#ifdef __cpp_lib_is_final
			!std::is_final<Type>::value
#else
			false
#endif
		>
		class empty_store_base {
			Type _value;
		public:
			_EX_CONSTEXPR_OVERLOAD Type& get()
			{
				return _value;
			}
			_EX_CONSTEXPR_OVERLOAD Type const& get() const
			{
				return _value;
			}
			template<typename... Args>
			constexpr empty_store_base(Args&& ... args):_value(std::forward<Args>(args)...)
			{}
			constexpr empty_store_base()
			{}
		};
		template<typename Type>
		class empty_store_base<Type,true>:Type {
		public:
			_EX_CONSTEXPR_OVERLOAD Type& get()
			{
				return *this;
			}
			_EX_CONSTEXPR_OVERLOAD Type const& get() const
			{
				return *this;
			}
			template<typename... Args>
			constexpr empty_store_base(Args&& ... args):Type(std::forward<Args>(args)...)
			{}
			constexpr empty_store_base(){}
		};
	}

	template<typename Type>
	class empty_store:public empty_store_impl::empty_store_base<Type> {
		using Base=empty_store_impl::empty_store_base<Type>;
	public:
		template<typename... Args>
		constexpr empty_store(Args&& ... args) noexcept(noexcept(Type(std::forward<Args>(args)...))):Base(std::forward<Args>(args)...)
		{}
		constexpr empty_store() noexcept(std::is_nothrow_default_constructible<Type>::value)
		{}
	};

#if _EXRETYPE_HAS_CPP_20
	using std::remove_cvref;
	using std::remove_cvref_t;
#else
	template<typename T>
	struct remove_cvref {
		using type=typename std::remove_cv<typename std::remove_reference<T>::type>::type;
	};

	template<typename T>
	using remove_cvref_t=typename remove_cvref<T>::type;
#endif

	/*
		Get the optimal param type for unmodified values (based on x64 calling convention)
		Suggested Usage: for simple non-virtual functions don't bother as it will be inlined anyway,
			for complex functions take in by T const& and forward to a helper that takes in const_param_type<T>::type
	*/
	template<typename T>
	struct const_param_type
		:std::conditional<
		std::is_trivially_copyable<T>::value&&sizeof(T)<=sizeof(std::size_t),
		T const,
		T const&> {};

	template<typename T>
	struct const_param_type<T[]> {
		using type=T* const;
	};

	template<typename T,std::size_t N>
	struct const_param_type<T[N]> {
		using type=T* const;
	};

	template<typename T>
	using const_param_type_t=typename const_param_type<T>::type;

	/*
		Get the optimal param type for moved values (based on x64 calling convention)
	*/
	template<typename T>
	struct move_param_type
		:std::conditional<
		std::is_trivially_copyable<T>::value&&sizeof(T)<=sizeof(std::size_t),
		T,
		T&&> {};

	template<typename T>
	using move_param_type_t=typename move_param_type<T>::type;

	//courtesy https://vittorioromeo.info/Misc/fwdlike.html
	template<typename Model,typename Orig>
	struct apply_value_category:std::conditional<std::is_lvalue_reference<Model>::value,
		typename std::remove_reference<Orig>::type&,
		typename std::remove_reference<Orig>::type&&> {};

	template<typename T,typename U>
	using apply_value_category_t=typename apply_value_category<T,U>::type;

	template<typename Model,typename Orig>
	constexpr apply_value_category_t<Model,Orig> forward_like(Orig&& orig) noexcept
	{
		return static_cast<apply_value_category_t<Model,Orig>>(orig);
	}

	//Given two types, gets the type that would make Original have the same const reference qualifiers as Model
	//T const&, U& -> U const&
	template<typename Model,typename Original>
	struct const_like {
		using type=Original;
	};

	template<typename T,typename U>
	struct const_like<T&,U&> {
		using type=U&;
	};

	template<typename T,typename U>
	struct const_like<T const&,U&> {
		using type=U const&;
	};

	template<typename T,typename U>
	using const_like_t=typename const_like<T,U>::type;

	//Strips cv qualifiers from reference and value types
	template<typename T>
	struct rvalue_reference_to_value {
		using type=typename std::remove_cv<T>::type;
	};

	//Removes rvalue_reference and strips cv qualifiers
	template<typename T>
	struct rvalue_reference_to_value<T&&>:rvalue_reference_to_value<T> {};

	//Removes rvalue_reference and then strips cv qualifiers
	template<typename T>
	using rvalue_reference_to_value_t=typename rvalue_reference_to_value<T>::type;

	namespace max_cref_impl {
		template<typename... Types>
		struct max_cref_impl;

		//return nothing
		template<>
		struct max_cref_impl<> {
			using type=void;
		};

		//return same type
		template<typename A>
		struct max_cref_impl<A> {
			using type=A;
		};

		//return same type
		template<typename A>
		struct max_cref_impl<A,A>:max_cref_impl<A> {};

		//utility to swap args to not repeat specializations
		template<typename A,typename B,bool same=std::is_same<remove_cvref_t<A>,remove_cvref_t<B>>::value>
		struct max_cref_swap {
			using type=typename max_cref_impl<B,A>::type;
		};

		//no type if not same type
		template<typename A,typename B>
		struct max_cref_swap<A,B,false> {};

		//utility to swap args to not repeat specializations, forward to special
		template<typename A,typename B>
		struct max_cref_impl<A,B>:max_cref_swap<A,B> {};

		//to const ref
		template<typename A>
		struct max_cref_impl<A const&,A&> {
			using type=A const&;
		};

		//decay to value
		template<typename A>
		struct max_cref_impl<A,A const&> {
			using type=A;
		};

		//decay to value
		template<typename A>
		struct max_cref_impl<A,A&> {
			using type=A;
		};

		//fold across variadic types
		template<typename A,typename B,typename... Rest>
		struct max_cref_impl<A,B,Rest...>:max_cref_impl<typename max_cref_impl<A,B>::type,Rest...> {};
	}

	/*
		Gets the type that preserves the constness of both referenced types to allowing forwarded returning of parameter types
		e.g. T&, T& -> T&
		T const&, T& -> T const&
		T, T const& -> T
		T&&, T& -> T
	*/
	template<typename... Types>
	struct max_cref:max_cref_impl::max_cref_impl<typename rvalue_reference_to_value<Types>::type...> {};

	template<typename A,typename B>
	using max_cref_t=typename max_cref<A,B>::type;

	namespace max_cpointer_impl {
		template<typename... Types>
		struct max_cpointer_impl {};

		template<typename T>
		struct max_cpointer_impl<T*> {
			using type=T*;
		};

		template<typename A>
		struct max_cpointer_impl<A const*,A*> {
			using type=A const*;
		};
		template<typename A>
		struct max_cpointer_impl<A*,A const*> {
			using type=A const*;
		};
		template<typename A>
		struct max_cpointer_impl<A const*,A const*> {
			using type=A const*;
		};
		template<typename A>
		struct max_cpointer_impl<A*,A*> {
			using type=A*;
		};

		template<typename A,typename B,typename... Rest>
		struct max_cpointer_impl<A,B,Rest...>:max_cpointer_impl<typename max_cpointer_impl<A,B>::type,Rest...> {

		};
	}

	/*
		Given two or more pointer types, returns a const preserving pointer type
		e.g. T*,T* -> T*
		T const*,T* -> T const*
		T const*,T const* -> T const*
	*/
	template<typename... Pointers>
	struct max_cpointer:max_cpointer_impl::max_cpointer_impl<Pointers...> {};

	template<typename... Pointers>
	using max_cpointer_t=typename max_cpointer<Pointers...>::type;


	template<typename T>
	struct wrap_reference {
		using type=T;
	};

	template<typename T>
	struct wrap_reference<T&> {
		using type=std::reference_wrapper<T>;
	};

	template<typename T>
	using wrap_reference_t=typename wrap_reference<T>::type;

	template<typename T>
	struct unwrap_reference_wrapper {
		using type=T;
	};

	template<typename T>
	struct unwrap_reference_wrapper<std::reference_wrapper<T>> {
		using type=T;
	};

	template<typename T>
	using unwrap_reference_wrapper_t=typename unwrap_reference_wrapper<T>::type;

	/*
		Allows both a pointer and reference to be passed, which will both decay to pointer semantics.
		Useful to allow "null references" while allowing call by reference.
	*/
	template<typename T>
	struct ref_transfer {
		T* _base;
	public:
		ref_transfer(ref_transfer const&)=delete;
		constexpr ref_transfer(T& obj) noexcept:_base(&obj)
		{}
		constexpr ref_transfer(T* obj) noexcept:_base(obj)
		{}
		constexpr operator T* () const noexcept
		{
			return _base;
		}
		constexpr T& operator*() const noexcept
		{
			return *_base;
		}
		constexpr T* operator->() const noexcept
		{
			return _base;
		}
		constexpr explicit operator bool() const noexcept
		{
			return _base;
		}
	};

	namespace detail {

		template<std::size_t N>
		struct match_size {
			using type=void;
		};
		template<>
		struct match_size<1> {
			using type=std::uint8_t;
		};

		template<>
		struct match_size<2> {
			using type=std::uint16_t;
		};

		template<>
		struct match_size<4> {
			using type=std::uint32_t;
		};

		template<>
		struct match_size<8> {
			using type=std::uint64_t;
		};

		template<typename T>
		struct suppress_void {
			using type=T;
		};
		template<>
		struct suppress_void<void> {};
	}

	/*
		Gets the unsigned integer with the given size.
	*/
	template<std::size_t N>
	struct match_size:detail::suppress_void<typename detail::match_size<N>::type> {
		static_assert(!std::is_same<typename detail::match_size<N>::type,void>::value,"No matching uint type of given size");
	};

	/*
		The unsigned integer type matching the given size.
	*/
	template<std::size_t N>
	using match_size_t=typename match_size<N>::type;

	namespace detail {

		template<std::size_t N>
		struct match_float_size_h {
			template<size_t Candidate=sizeof(long double),typename... Extra>
			static auto try_long_double(Extra...) -> typename std::enable_if<Candidate==N,long double>::type;
			template<size_t=sizeof(long double),typename... Extra>
			static void try_long_double(Extra...);

			template<size_t Candidate=sizeof(double)>
			static auto try_double() -> typename std::enable_if<Candidate==N,double>::type;
			template<size_t=sizeof(float),typename... Extra>
			static auto try_double(Extra...) -> decltype(try_long_double());

			template<size_t Candidate=sizeof(float)>
			static auto try_float() -> typename std::enable_if<Candidate==N,float>::type;
			template<size_t=sizeof(float),typename... Extra>
			static auto try_float(Extra...) -> decltype(try_double());

			using type=decltype(try_float());
		};

		template<std::size_t N>
		struct least_float_size_h {
			template<size_t Candidate=sizeof(long double),typename... Extra>
			static auto try_long_double(Extra...) -> typename std::enable_if<Candidate>=N,long double>::type;
			template<size_t=sizeof(long double),typename... Extra>
			static void try_long_double(Extra...);

			template<size_t Candidate=sizeof(double)>
			static auto try_double() -> typename std::enable_if<Candidate>=N,double>::type;
			template<size_t=sizeof(float),typename... Extra>
			static auto try_double(Extra...) -> decltype(try_long_double());

			template<size_t Candidate=sizeof(float)>
			static auto try_float() -> typename std::enable_if<Candidate>=N,float>::type;
			template<size_t=sizeof(float),typename... Extra>
			static auto try_float(Extra...) -> decltype(try_double());

			using type=decltype(try_float());
		};

		template<std::size_t N>
		struct least_size_h {
			template<size_t Candidate=sizeof(uint64_t)>
			static auto try64() -> typename std::enable_if<Candidate>=N,uint64_t>::type;
			template<size_t=sizeof(uint64_t),typename... Extra>
			static void try64(Extra...);

			template<size_t Candidate=sizeof(uint32_t)>
			static auto try32() -> typename std::enable_if<Candidate>=N,uint32_t>::type;
			template<size_t=sizeof(uint32_t),typename... Extra>
			static auto try32(Extra...) -> decltype(try64());

			template<size_t Candidate=sizeof(uint16_t)>
			static auto try16() -> typename std::enable_if<Candidate>=N,uint16_t>::type;
			template<size_t=sizeof(uint16_t),typename... Extra>
			static auto try16(Extra...) -> decltype(try32());

			template<size_t Candidate=sizeof(uint8_t)>
			static auto try8() -> typename std::enable_if<Candidate>=N,uint8_t>::type;
			template<size_t Candidate=sizeof(uint8_t),typename... Extra>
			static auto try8(Extra...) -> decltype(try16());

			using type=decltype(try8());
		};
	}

	/*
		Gets the floating point data type with same size as given amount.
	*/
	template<std::size_t N>
	struct match_float_size:detail::suppress_void<typename detail::match_float_size_h<N>::type> {};

	/*
		Floating point data type with same size as given amount.
	*/
	template<std::size_t N>
	using match_float_size_t=typename match_float_size<N>::type;

	/*
		Gets the floating point data type with size at least given amount.
	*/
	template<std::size_t N>
	struct least_float_size:detail::suppress_void<typename detail::least_float_size_h<N>::type> {};

	/*
		Floating point data type with size at least given amount.
	*/
	template<std::size_t N>
	using least_float_size_t=typename least_float_size<N>::type;

	/*
		Gets the unsigned integer data type with size at least given amount.
	*/
	template<std::size_t N>
	struct least_size:detail::least_size_h<N> {};

	/*
		Unsigned integer data type with size at least given amount.
	*/
	template<std::size_t N>
	using least_size_t=typename least_size<N>::type;

	template<typename Forwardee>
	class forward_string {
		using value_type=typename Forwardee::value_type;
		using pointer=value_type const*;
		value_type const* data;
	public:
		forward_string(Forwardee const& f):data(f.c_str())
		{}
		constexpr forward_string(value_type const* data):data(data)
		{}
		constexpr operator pointer const& () const
		{
			return data;
		}
		constexpr operator pointer& ()
		{
			return data;
		}
	};

	namespace detail {
		template<typename Func>
		struct wrap {
			template<typename F>
			static constexpr F get(F&& f) noexcept
			{
				return std::forward<F>(f);
			}
		};
#if !_EXRETYPE_HAS_CPP_14
		template<typename T>
		class func_pointer_wrapper;

		template<typename Ret,typename... Args>
		class func_pointer_wrapper<Ret(*)(Args...)> {
			Ret(*f)(Args...);
		public:
			func_pointer_wrapper(Ret(*f)(Args...)):f(f)
			{}
			Ret operator()(Args... args) const
			{
				return f(std::forward<Args>(args)...);
			}
		};

		template<typename Ret,typename... Args>
		class wrap<Ret(*)(Args...)> {
			static constexpr func_pointer_wrapper<Ret(*)(Args...)> get(Ret(*f)(Args...))
			{
				return {f};
			}
		};
		template<typename Ret,typename... Args>
		struct wrap<Ret(Args...)> {
			static constexpr func_pointer_wrapper<Ret(*)(Args...)> get(Ret(&f)(Args...))
			{
				return {&f};
			}
		};
#else
		template<typename Ret,typename... Args>
		struct wrap<Ret(*)(Args...)> {
			static constexpr auto get(Ret(*f)(Args...)) noexcept
			{
				return [f](Args... args) -> Ret
				{
					return (f(std::forward<Args>(args)...));
				};
			}
		};
		template<typename Ret,typename... Args>
		struct wrap<Ret(Args...)> {
			static constexpr auto get(Ret(&f)(Args...)) noexcept
			{
				return [f](Args... args) -> Ret
				{
					return (f(std::forward<Args>(args)...));
				};
			}
		};
#endif

	}

	//wraps a function pointer (or really anything callable) in a lambda
	template<typename Func>
	constexpr auto wrap(Func&& fp) noexcept -> decltype(detail::wrap<remove_cvref_t<Func>>::get(std::forward<Func>(fp)))
	{
		return detail::wrap<remove_cvref_t<Func>>::get(std::forward<Func>(fp));
	}


#ifdef __cpp_variadic_using
	template<typename... Funcs>
	struct overloaded:private Funcs...
	{
		template<typename... F>
		overloaded(F&&... f):Funcs(wrap(std::forward<F>(f)))... {
		}
		using Funcs::operator()...;
	};

#else
	template<typename... Funcs>
	struct overloaded;

	template<>
	struct overloaded<> {};

	template<typename First>
	struct overloaded<First>:private First{
		using First::operator();
		template<typename F>
		overloaded(F&& f):First{wrap(std::forward<F>(f))}{}
	};

	template<typename First,typename... Rest>
	struct overloaded<First,Rest...>:private First,private overloaded<Rest...> {
	private:
		using ORest=overloaded<Rest...>;
	public:
		using First::operator();
		using ORest::operator();
		template<typename F,typename... R>
		overloaded(F&& f,R&&... r):First{wrap(std::forward<F>(f))},ORest{std::forward<R>(r)...}{}
	};
#endif

	template<typename... Funcs>
	constexpr auto make_overloaded(Funcs&&... f) -> decltype(overloaded<remove_cvref_t<decltype(wrap(f))>...>(std::forward<Funcs>(f)...))
	{
		return overloaded<remove_cvref_t<decltype(wrap(f))>...>(std::forward<Funcs>(f)...);
	}

#ifdef __cpp_deduction_guides
	template<typename... Funcs>
	overloaded(Funcs&& ... f)->overloaded<remove_cvref_t<decltype(wrap(f))>...>;
#endif

	struct empty_t{};

	template<typename T>
	struct is_sized_array:
		std::conditional<
			std::is_same<T,remove_cvref_t<T>>::value,
			std::false_type,
			is_sized_array<remove_cvref_t<T>>
		>::type {};

	template<typename T,std::size_t N>
	struct is_sized_array<std::array<T,N>>:std::true_type {};

	template<typename T, std::size_t N>
	struct is_sized_array<T[N]>:std::true_type {};

#if _EXRETYPE_HAS_CPP_20
	template<typename T,std::size_t N>
	struct is_sized_array<std::span<T,N>>:std::true_type {};

	template<typename T>
	struct is_sized_array<std::span<T,std::size_t(-1)>>:std::false_type {};
#endif

	template<typename T>
	struct array_size:std::conditional<std::is_same<T,remove_cvref_t<T>>::value,empty_t,array_size<remove_cvref_t<T>>>::type {};

	template<typename T,size_t N>
	struct array_size<std::array<T,N>>:std::integral_constant<size_t,N> {};

	template<typename T,size_t N>
	struct array_size<T[N]>:std::integral_constant<size_t,N> {};

#if _EXRETYPE_HAS_CPP_20
	template<typename T,std::size_t N>
	struct array_size<std::span<T,N>>:std::integral_constant<size_t,N> {};

	template<typename T>
	struct array_size<std::span<T,std::size_t(-1)>> {};
#endif

#ifdef __cpp_inline_variables
	template<typename T>
	inline constexpr size_t array_size_v=array_size<T>::value;
#endif

	template<typename T>
	struct array_type:std::conditional<std::is_same<T,remove_cvref_t<T>>::value,empty_t,array_size<remove_cvref_t<T>>>::type {};

	template<typename T,size_t N>
	struct array_type<std::array<T,N>> {
		using type=T;
	};

	template<typename T,size_t N>
	struct array_type<T[N]> {
		using type=T;
	};

	template<typename T>
	struct array_type<T[]> {
		using type=T;
	};

#if _EXRETYPE_HAS_CPP_20
	template<typename T,size_t N>
	struct array_type<std::span<T,N>> {
		using type=T;
	};
#endif

	template<typename Arr>
	using array_type_t=typename array_type<Arr>::type;

	template<typename SumType,typename... Constants>
	struct sum_type_value:std::integral_constant<SumType,0>{};

	template<typename SumType,typename Type>
	struct sum_type_value<SumType,Type>:std::integral_constant<SumType,Type::value> {
	};

	template<typename SumType,typename Type,typename... Rest>
	struct sum_type_value<SumType,Type,Rest...>:std::integral_constant<SumType,Type::value+sum_type_value<SumType,Rest...>::value> {
	};

#ifdef __cpp_lib_logical_traits
	using std::conjunction;
#else
	template<typename... Types>
	struct conjunction;

	template<>
	struct conjunction<>:std::true_type{};

	template<typename Type>
	struct conjunction<Type>:std::integral_constant<bool,bool(Type::value)>{};

	template<typename First,typename... Rest>
	struct conjunction<First,Rest...>:std::integral_constant<bool,bool(First::value)&&conjunction<Rest...>::value> {};
#endif


#ifdef __cpp_lib_integer_sequence
	using std::index_sequence;

	using std::make_index_sequence;
#else
	template<size_t... Is>
	struct index_sequence {};

	namespace detail{
		template<typename T,typename U>
		struct concat_index_sequence;

		template<size_t... I,size_t... J>
		struct concat_index_sequence<index_sequence<I...>,index_sequence<J...>> {
			using type=index_sequence<I...,J...>;
		};

		template<typename T,typename U>
		using concat_index_sequence_t=typename concat_index_sequence<T,U>::type;

		template<size_t I,size_t J,bool adjacent=I+1==J>
		struct make_index_sequence_h {
			static constexpr size_t H=I+(J-I)/2;
			using type=concat_index_sequence_t<typename make_index_sequence_h<I,H>::type,typename make_index_sequence_h<H,J>::type>;
		};

		template<size_t I>
		struct make_index_sequence_h<I,I,false> {
			using type=index_sequence<>;
		};

		template<size_t I,size_t J>
		struct make_index_sequence_h<I,J,true> {
			using type=index_sequence<I>;
		};
	}

	template<size_t N>
	using make_index_sequence=typename detail::make_index_sequence_h<0,N>::type;
#endif
}
#endif