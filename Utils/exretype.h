#pragma once
/*
Contains utilities for combining and recreating types.
*/
#ifndef EXRETYPE_H
#define EXRETYPE_H
#include <type_traits>
namespace exlib {

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

	template<typename... Funcs>
	struct overloaded:private Funcs...
	{
		template<typename... F>
		overloaded(F&&... f):Funcs(wrap(std::forward<F>(f)))...{
		}
		using Funcs::operator()...;
	};

	template<typename... Funcs>
	constexpr auto make_overloaded(Funcs&&... f)
	{
		return overloaded<std::remove_cv_t<std::remove_reference_t<decltype(wrap(f))>>...>(std::forward<Funcs>(f)...);
	}

	#if __cplusplus > 201700L || defined(_CRT_HAS_CXX17)
	template<typename... Funcs>
	overloaded(Funcs&&... f)->overloaded<std::remove_cv_t<std::remove_reference_t<decltype(wrap(f))>>...>;
#endif

	namespace detail {
		template<typename Base>
		class Box {
			Base _base;
		public:
			constexpr Box(Base b):_base(b){}
			constexpr Box(){}
			constexpr Box(Box const&)=default;
			constexpr Box& operator=(Box const&)=default;
			constexpr operator Base&()
			{
				return _base;
			}
			constexpr operator Base const&() const
			{
				return _base;
			}
#define make_bin_op_for_box(op)\
constexpr Box operator op(Box const& o) const { return {_base ##op## o._base};}\
constexpr Box& operator ##op##=(Box const& o) { _base ##op##= o._base; return *this;}
			make_bin_op_for_box(+)
				make_bin_op_for_box(-)
				make_bin_op_for_box(/)
				make_bin_op_for_box(*)
				make_bin_op_for_box(%)
				make_bin_op_for_box(|)
				make_bin_op_for_box(&)
				make_bin_op_for_box(^)
#undef make_bin_op_for_box
#define make_comp_op_for_box(op)\
constexpr friend bool operator##op##(Box a,Box b) {return a._base ##op## b._base;}\
constexpr friend bool operator##op##(Box a,Base b) {return a._base ##op## b;}\
constexpr friend bool operator##op##(Base a,Box b) {return a ##op## b._base;}
				make_comp_op_for_box(<)
				make_comp_op_for_box(>)
				make_comp_op_for_box(==)
				make_comp_op_for_box(<=)
				make_comp_op_for_box(>=)
				make_comp_op_for_box(!=)
#undef make_comp_op_for_box
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