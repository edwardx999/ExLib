#pragma once
/*
Contains utilities for combining and recreating types.
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
		constexpr ref_transfer(T& obj):_base(&obj){}
		constexpr ref_transfer(T* obj):_base(&obj){}
		constexpr operator T*() const
		{
			return _base;
		}
		constexpr T& operator*() const
		{
			return *_base;
		}
		constexpr T* operator->() const
		{
			return _base;
		}
	};
	/*
		Gets the unsigned integer with the given size.
	*/
	template<size_t N>
	struct match_size;

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

	template<size_t N>
	using match_size_t=typename match_size<N>::type;

	template<typename Forwardee>
	class forward_string {
		using value_type=typename Forwardee::value_type;
		using pointer=value_type const*;
		value_type const* data;
	public:
		forward_string(Forwardee const& f):data(f.c_str())
		{
		}
		constexpr forward_string(value_type const* data):data(data){}
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
			static constexpr decltype(auto) get(F&& f)
			{
				return std::forward<F>(f);
			}
		};

		template<typename Ret,typename... Args>
		struct wrap<Ret(*)(Args...)> {
			template<typename F>
			static constexpr auto get(F f)
			{
				return [f](Args... args) -> Ret
				{
					return (f(std::forward<Args>(args)...));
				};
			}
		};
		template<typename Ret,typename... Args>
		struct wrap<Ret(Args...)>:wrap<Ret(*)(Args...)> {};
	}

	//wraps a function pointer (or really anything callable) in a lambda
	template<typename Func>
	constexpr decltype(auto) wrap(Func&& fp)
	{
		return detail::wrap<std::remove_cv_t<std::remove_reference_t<Func>>>::get(std::forward<Func>(fp));
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
	constexpr auto make_overloaded(Funcs&&... f)
	{
		return overloaded<std::remove_cv_t<std::remove_reference_t<decltype(wrap(f))>>...>(std::forward<Funcs>(f)...);
	}

#if _EXRETYPE_HAS_CPP_17
	template<typename... Funcs>
	overloaded(Funcs&&... f)->overloaded<std::remove_cv_t<std::remove_reference_t<decltype(wrap(f))>>...>;
#endif

	namespace detail {
		template<typename Base>
		class Box {
			Base _base;
		public:
			constexpr Box(Base b):_base(b) {}
			constexpr Box() {}
			constexpr Box(Box const&)=default;
			constexpr Box& operator=(Box const&)=default;
			template<typename O> constexpr Box& operator=(O const& o)
			{
				_base=o;
				return *this;
			}
			constexpr operator Base&()
			{
				return _base;
			}
			constexpr operator Base const&() const
			{
				return _base;
			}
#define make_bin_op_for_box(op)\
constexpr Box operator op(Box o) const { return {_base ##op## o._base}; }\
constexpr Box& operator ##op##=(Box o) { _base ##op##= o._base; return *this;}\
template<typename O> constexpr Box operator op(O const& o) const { return {_base ##op## o}; }\
template<typename O> constexpr Box& operator ##op##=(O const& o) { _base ##op##= o; return *this;}
			make_bin_op_for_box(+)
				make_bin_op_for_box(-)
				make_bin_op_for_box(/)
				make_bin_op_for_box(*)
				make_bin_op_for_box(%)
				make_bin_op_for_box(|)
				make_bin_op_for_box(&)
				make_bin_op_for_box(^)
#undef make_bin_op_for_box
				constexpr Box operator~() const
			{
				return {~_base};
			}
			constexpr operator bool() const
			{
				return _base;
			}
			constexpr bool operator!() const
			{
				return !_base;
			}
			constexpr Box operator+() const
			{
				return *this;
			}
			constexpr Box operator-() const
			{
				return {-_base};
			}
#define make_shift_op_for_box(op)\
constexpr Box operator##op##(size_t c) const {return {_base ##op## c};}\
			constexpr Box operator##op##=(size_t c) { return _base ##op##=c,*this; }
			make_shift_op_for_box(<<)
				make_shift_op_for_box(>>)
#undef make_shift_op_for_box
				constexpr Box operator++(int)
			{
				auto c(*this);
				++_base;
				return c;
			}
			constexpr Box operator--(int)
			{
				auto c(*this);
				--_base;
				return c;
			}
			constexpr Box& operator++()
			{
				++_base;
				return *this;
			}
			constexpr Box& operator--()
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
template<typename Base> constexpr bool operator##op##(Box<Base> a,Box<Base> b) {return static_cast<Base&>(a) ##op## static_cast<Base&>(b);}\
template<typename Base,typename O> constexpr bool operator##op##(Box<Base> a,O b) {return static_cast<Base&>(a) ##op## b;}\
template<typename O,typename Base> constexpr bool operator##op##(O a,Box<Base> b) {return a ##op## static_cast<Base&>(b);}
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