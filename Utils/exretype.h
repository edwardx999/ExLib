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
#define _EXRETYPE_HAS_CPP_20 _MSVC_LANG>=202000l
#define _EXRETYPE_HAS_CPP_17 _MSVC_LANG>=201700l
#define _EXRETYPE_HAS_CPP_14 _MSVC_LANG>=201400l
#else
#define _EXRETYPE_HAS_CPP_20 __cplusplus>=202000l
#define _EXRETYPE_HAS_CPP_17 __cplusplus>=201700l
#define _EXRETYPE_HAS_CPP_14 __cplusplus>=201400l
#endif
#include <type_traits>
#include <cstdint>
namespace exlib {

	template<typename... Bases>
	struct combined:Bases...{

	};

	namespace empty_store_impl {
		template<typename Type,bool ShouldInherit=std::is_empty<Type>::value>
		class empty_store_base {
			Type _value;
		public:
			Type& get()
			{
				return _value;
			}
			Type const& get() const
			{
				return _value;
			}
			template<typename... Args>
			empty_store_base(Args&& ... args):_value(std::forward<Args>(args)...)
			{}
			empty_store_base()
			{}
		};
		template<typename Type>
		class empty_store_base<Type,true>:Type {
		public:
			Type& get()
			{
				return *this;
			}
			Type const& get() const
			{
				return *this;
			}
			template<typename... Args>
			empty_store_base(Args&& ... args):Type(std::forward<Args>(args)...)
			{}
			empty_store_base(){}
		};
	}

	template<typename Type>
	class empty_store:public empty_store_impl::empty_store_base<Type> {
		using Base=empty_store_impl::empty_store_base<Type>;
	public:
		template<typename... Args>
		empty_store(Args&& ... args) noexcept(noexcept(Type(std::forward<Args>(args)...))):Base(std::forward<Args>(args)...)
		{}
		empty_store() noexcept(std::is_nothrow_default_constructible<Type>::value)
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
		std::is_trivially_copyable<T>::value&&sizeof(T)<=sizeof(size_t),
		T const,
		T const&> {};

	template<typename T>
	struct const_param_type<T[]> {
		using type=T*const;
	};

	template<typename T,size_t N>
	struct const_param_type<T[N]> {
		using type=T*const;
	};

	template<typename T>
	using const_param_type_t=typename const_param_type<T>::type;

	//courtesy https://vittorioromeo.info/Misc/fwdlike.html
	template<typename Model,typename Orig>
	struct apply_value_category:std::conditional<std::is_lvalue_reference_v<Model>,
		std::remove_reference_t<Orig>&,
		std::remove_reference_t<Orig>&&> {};

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
		struct max_cpointer_impl;

		template<>
		struct max_cpointer_impl<> {
			using type=void;
		};

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
		T& _base;
	public:
		ref_transfer(ref_transfer const&)=delete;
		constexpr ref_transfer(T& obj) noexcept:_base(obj)
		{}
		constexpr ref_transfer(T* obj) noexcept:_base(*obj)
		{}
		constexpr operator T* () const noexcept
		{
			return &_base;
		}
		constexpr T& operator*() const noexcept
		{
			return _base;
		}
		constexpr T* operator->() const noexcept
		{
			return &_base;
		}
		constexpr explicit operator bool() const noexcept
		{
			return &_base;
		}
	};


	namespace detail {

		template<size_t N>
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
	template<size_t N>
	struct match_size:detail::suppress_void<typename detail::match_size<N>::type> {
		static_assert(!std::is_same<typename detail::match_size<N>::type,void>::value,"No matching uint type of given size");
	};

	/*
		The unsigned integer type matching the given size.
	*/
	template<size_t N>
	using match_size_t=typename match_size<N>::type;

	namespace detail {

		template<size_t N>
		struct match_float_size_h {
			template<typename... Extra>
			static auto try_long_double(int,Extra...) -> decltype(std::enable_if<sizeof(long double)==N,long double>::type());
			template<typename... Extra>
			static void try_long_double(char,Extra...);

			template<typename... Extra>
			static auto try_double(int,Extra...) -> decltype(std::enable_if<sizeof(double)==N,double>::type());
			template<typename... Extra>
			static auto try_double(char,Extra...) -> decltype(try_long_double(0));

			template<typename... Extra>
			static auto try_float(int,Extra...) -> decltype(std::enable_if<sizeof(float)==N,float>::type());
			template<typename... Extra>
			static auto try_float(char,Extra...) -> decltype(try_double(0));

			using type=decltype(try_float(0));
			static_assert(!std::is_same<type,void>::value,"No floating-point type matching size");
		};

		template<size_t N>
		struct least_float_size_h {
			template<typename... Extra>
			static auto try_long_double(int,Extra...) -> decltype(std::enable_if<sizeof(long double)>=N,long double>::type());
			template<typename... Extra>
			static void try_long_double(char,Extra...);

			template<typename... Extra>
			static auto try_double(int,Extra...) -> decltype(std::enable_if<sizeof(double)>=N,double>::type());
			template<typename... Extra>
			static auto try_double(char,Extra...) -> decltype(try_long_double(0));

			template<typename... Extra>
			static auto try_float(int,Extra...) -> decltype(std::enable_if<sizeof(float)>=N,float>::type());
			template<typename... Extra>
			static auto try_float(char,Extra...) -> decltype(try_double(0));

			using type=decltype(try_float(0));
			static_assert(!std::is_same<type,void>::value,"No floating-point type at least size");
		};

		template<size_t N>
		struct least_size_h {
			template<typename... Extra>
			static auto try64(int,Extra...) -> decltype(std::enable_if<sizeof(uint64_t)>=N,uint64_t>::type());
			template<typename... Extra>
			static void try64(char,Extra...);

			template<typename... Extra>
			static auto try32(int,Extra...) -> decltype(std::enable_if<sizeof(uint32_t)>=N,uint32_t>::type());
			template<typename... Extra>
			static auto try32(char,Extra...) -> decltype(try64(0));

			template<typename... Extra>
			static auto try16(int,Extra...) -> decltype(std::enable_if<sizeof(uint16_t)>=N,uint16_t>::type());
			template<typename... Extra>
			static auto try16(char,Extra...) -> decltype(try32(0));

			template<typename... Extra>
			static auto try8(int,Extra...) -> decltype(std::enable_if<sizeof(uint8_t)>=N,uint8_t>::type());
			template<typename... Extra>
			static auto try8(char,Extra...) -> decltype(try16(0));

			using type=decltype(try8(0));
			static_assert(!std::is_same<type,void>::value,"No uint type at least size");
		};
	}

	/*
		Gets the floating point data type with same size as given amount.
	*/
	template<size_t N>
	struct match_float_size:detail::suppress_void<typename detail::match_float_size_h<N>::type> {};

	/*
		Floating point data type with same size as given amount.
	*/
	template<size_t N>
	using match_float_size_t=typename match_float_size<N>::type;

	/*
		Gets the floating point data type with size at least given amount.
	*/
	template<size_t N>
	struct least_float_size:detail::suppress_void<typename detail::least_float_size_h<N>::type> {};

	/*
		Floating point data type with size at least given amount.
	*/
	template<size_t N>
	using least_float_size_t=typename least_float_size<N>::type;

	/*
		Gets the unsigned integer data type with size at least given amount.
	*/
	template<size_t N>
	struct least_size:detail::least_size_h<N> {};

	/*
		Unsigned integer data type with size at least given amount.
	*/
	template<size_t N>
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


#if _EXRETYPE_HAS_CPP_17
	template<typename... Funcs>
	struct overloaded:private Funcs...
	{
		template<typename... F>
		overloaded(F&& ... f):Funcs(wrap(std::forward<F>(f)))...{
		}
		using Funcs::operator()...;
	};

#else
	template<typename... Funcs>
	struct overloaded:public Funcs...
	{
		template<typename... F>
		overloaded(F&& ... f):Funcs(wrap(std::forward<F>(f)))...{
		}
	};
#endif

	template<typename... Funcs>
	constexpr auto make_overloaded(Funcs&& ... f) -> decltype(overloaded<remove_cvref_t<decltype(wrap(f))>...>(std::forward<Funcs>(f)...))
	{
		return overloaded<remove_cvref_t<decltype(wrap(f))>...>(std::forward<Funcs>(f)...);
	}

#if _EXRETYPE_HAS_CPP_17
	template<typename... Funcs>
	overloaded(Funcs&& ... f)->overloaded<remove_cvref_t<decltype(wrap(f))>...>;
#endif

	namespace detail {
		template<typename Base>
		class Box {
			Base _base;
		public:
			constexpr Box(Base b) noexcept:_base(b)
			{}
			constexpr Box() noexcept
			{}
			constexpr Box(Box const&)=default;
			constexpr Box& operator=(Box const&)=default;
			template<typename O> constexpr Box& operator=(O const& o) noexcept
			{
				_base=o;
				return *this;
			}
			constexpr operator Base& () noexcept
			{
				return _base;
			}
			constexpr operator Base const& () const noexcept
			{
				return _base;
			}
#define make_bin_op_for_box(op)\
constexpr Box operator ##op##(Box o) const noexcept { return {_base ##op## o._base}; }\
constexpr Box& operator ##op##=(Box o) noexcept { _base ##op##= o._base; return *this;}\
template<typename O> constexpr Box operator ##op##(O const& o) const noexcept { return {_base ##op## o}; }\
template<typename O> constexpr Box& operator ##op##=(O const& o) noexcept { _base ##op##= o; return *this;}
			make_bin_op_for_box(+)
				make_bin_op_for_box(-)
				make_bin_op_for_box(/)
				make_bin_op_for_box(*)
				make_bin_op_for_box(%)
				make_bin_op_for_box(|)
				make_bin_op_for_box(&)
				make_bin_op_for_box(^)
#undef make_bin_op_for_box
				constexpr Box operator~() const noexcept
			{
				return {~_base};
			}
			constexpr operator bool() const noexcept
			{
				return _base;
			}
			constexpr bool operator!() const noexcept
			{
				return !_base;
			}
			constexpr Box operator+() const noexcept
			{
				return *this;
			}
			constexpr Box operator-() const noexcept
			{
				return {-_base};
			}
#define make_shift_op_for_box(op)\
constexpr Box operator##op##(size_t c) const noexcept {return {_base ##op## c};}\
			constexpr Box operator##op##=(size_t c) noexcept { return _base ##op##=c,*this; }
			make_shift_op_for_box(<<)
				make_shift_op_for_box(>>)
#undef make_shift_op_for_box
				constexpr Box operator++(int) noexcept
			{
				auto c(*this);
				++_base;
				return c;
			}
			constexpr Box operator--(int) noexcept
			{
				auto c(*this);
				--_base;
				return c;
			}
			constexpr Box& operator++() noexcept
			{
				++_base;
				return *this;
			}
			constexpr Box& operator--() noexcept
			{
				--_base;
				return *this;
			}
		};

		template<typename OStream,typename Base>
		OStream& operator<<(OStream& os,Box<Base> b)
		{
			return os<<static_cast<Base&>(b);
		}

#define make_comp_op_for_box(op)\
template<typename Base> constexpr bool operator##op##(Box<Base> a,Box<Base> b) noexcept {return static_cast<Base&>(a) ##op## static_cast<Base&>(b);}\
template<typename Base,typename O> constexpr bool operator##op##(Box<Base> a,O b) noexcept {return static_cast<Base&>(a) ##op## b;}\
template<typename O,typename Base> constexpr bool operator##op##(O a,Box<Base> b) noexcept {return a ##op## static_cast<Base&>(b);}
		make_comp_op_for_box(<)
			make_comp_op_for_box(>)
			make_comp_op_for_box(==)
			make_comp_op_for_box(<=)
			make_comp_op_for_box(>=)
			make_comp_op_for_box(!=)
#undef make_comp_op_for_box
	}

	using Char=detail::Box<char>;
	using UnsignedChar=detail::Box<unsigned char>;
	using SignedChar=detail::Box<signed char>;

	using WChar=detail::Box<wchar_t>;

	using Short=detail::Box<short>;
	using UnsignedShort=detail::Box<unsigned short>;
	using SignedShort=detail::Box<signed short>;

	using Int=detail::Box<int>;
	using UnsignedInt=detail::Box<unsigned int>;
	using SignedInt=detail::Box<signed int>;

	using Long=detail::Box<long>;
	using UnsignedLong=detail::Box<unsigned long>;
	using SignedLong=detail::Box<signed long>;

	using LongLong=detail::Box<long long>;
	using UnsignedLongLong=detail::Box<unsigned long long>;
	using SignedLongLong=detail::Box<signed long long>;

	using Double=detail::Box<double>;
	using Float=detail::Box<float>;

	using Char16=detail::Box<char16_t>;
	using Char32=detail::Box<char32_t>;
#if _EXRETYPE_HAS_CPP_20
	using Char8=detail::Box<char8_t>;
#endif

	template<typename T>
	struct get_box {
		using type=T;
	};

#define get_box_specialization(ttype)\
	template<>\
	struct get_box<ttype> {\
		using type=detail::Box<ttype>;\
	}
	get_box_specialization(char);
	get_box_specialization(signed char);
	get_box_specialization(unsigned char);
	get_box_specialization(short);
	get_box_specialization(unsigned short);
	get_box_specialization(int);
	get_box_specialization(unsigned int);
	get_box_specialization(long);
	get_box_specialization(unsigned long);
	get_box_specialization(long long);
	get_box_specialization(unsigned long long);
	get_box_specialization(float);
	get_box_specialization(double);
	get_box_specialization(wchar_t);
	get_box_specialization(char16_t);
	get_box_specialization(char32_t);
#if _EXRETYPE_HAS_CPP_20
	get_box_specialization(char8_t);
#endif 
#undef get_box_specialization

	template<typename T>
	using get_box_t=typename get_box<T>::type;


}
#endif