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
#define _EXRETYPE_HAS_CPP_20 _MSVC_LANG>202000l
#define _EXRETYPE_HAS_CPP_17 _MSVC_LANG>201700l
#define _EXRETYPE_HAS_CPP_14 _MSVC_LANG>201700l
#else
#define _EXRETYPE_HAS_CPP_20 __cplusplus>202000l
#define _EXRETYPE_HAS_CPP_17 __cplusplus>201700l
#define _EXRETYPE_HAS_CPP_14 __cplusplus>201700l
#endif
#include <type_traits>
#include <cstdint>
namespace exlib {

	/*
		Allows both a pointer and reference to be passed, which will both decay to pointer semantics.
		Useful to allow "references" while allowing call by reference.
	*/
	template<typename T>
	struct ref_transfer {
		T* _base;
	public:
		constexpr ref_transfer(T& obj):_base(&obj) {}
		constexpr ref_transfer(T* obj):_base(obj) {}
		constexpr operator T*() const noexcept
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
		constexpr forward_string(value_type const* data):data(data) {}
		constexpr operator pointer const&() const
		{
			return data;
		}
		constexpr operator pointer&()
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
			func_pointer_wrapper(Ret(*f)(Args...)):f(f) {}
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
				return [&f](Args... args) -> Ret
				{
					return (f(std::forward<Args>(args)...));
				};
			}
		};
#endif
#if _EXRETYPE_HAS_CPP_20
		using std::remove_cvref_t;
#else
		template<typename T>
		using remove_cvref_t=typename std::remove_cv<typename std::remove_reference<T>::type>::type;
#endif
	}

	//wraps a function pointer (or really anything callable) in a lambda
	template<typename Func>
	constexpr auto wrap(Func&& fp) noexcept -> decltype(detail::wrap<detail::remove_cvref_t<Func>>::get(std::forward<Func>(fp)))
	{
		return detail::wrap<detail::remove_cvref_t<Func>>::get(std::forward<Func>(fp));
	}


#if _EXRETYPE_HAS_CPP_17
	template<typename... Funcs>
	struct overloaded:private Funcs...
	{
		template<typename... F>
		overloaded(F&&... f):Funcs(wrap(std::forward<F>(f)))...{
		}
		using Funcs::operator()...;
	};

#else
	template<typename... Funcs>
	struct overloaded:public Funcs...
	{
		template<typename... F>
		overloaded(F&&... f):Funcs(wrap(std::forward<F>(f)))...{
		}
	};
#endif

	template<typename... Funcs>
	constexpr auto make_overloaded(Funcs&&... f) -> decltype(overloaded<detail::remove_cvref_t<decltype(wrap(f))>...>(std::forward<Funcs>(f)...))
	{
		return overloaded<detail::remove_cvref_t<decltype(wrap(f))>...>(std::forward<Funcs>(f)...);
	}

#if _EXRETYPE_HAS_CPP_17
	template<typename... Funcs>
	overloaded(Funcs&&... f)->overloaded<detail::remove_cvref_t<decltype(wrap(f))>...>;
#endif

	namespace detail {
		template<typename Base>
		class Box {
			Base _base;
		public:
			constexpr Box(Base b) noexcept:_base(b) {}
			constexpr Box() noexcept {}
			constexpr Box(Box const&)=default;
			constexpr Box& operator=(Box const&)=default;
			template<typename O> constexpr Box& operator=(O const& o) noexcept
			{
				_base=o;
				return *this;
			}
			constexpr operator Base&() noexcept
			{
				return _base;
			}
			constexpr operator Base const&() const noexcept
			{
				return _base;
			}
#define make_bin_op_for_box(op)\
constexpr Box operator op(Box o) const noexcept { return {_base ##op## o._base}; }\
constexpr Box& operator ##op##=(Box o) noexcept { _base ##op##= o._base; return *this;}\
template<typename O> constexpr Box operator op(O const& o) noexcept const { return {_base ##op## o}; }\
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
#undef get_box_specialization

	template<typename T>
	using get_box_t=typename get_box<T>::type;


}
#endif