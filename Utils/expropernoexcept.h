#pragma once
#ifndef EX_PROPERNOEXCEPT_H
#define EX_PROPERNOEXCEPT_H
#include <type_traits>
namespace exlib {

	template<typename T>
	class properly_noexcept_wrapper:public T {
	public:
		using T::T;
		constexpr properly_noexcept_wrapper() noexcept
		{}
		constexpr properly_noexcept_wrapper(properly_noexcept_wrapper&& o) noexcept:T(static_cast<T&&>(o))
		{}
		constexpr properly_noexcept_wrapper& operator=(properly_noexcept_wrapper&& o) noexcept
		{
			static_cast<T&>(*this)=static_cast<T&&>(o);
		}
		~properly_noexcept_wrapper() noexcept
		{}

		template<typename OTHER>
		constexpr auto swap(OTHER& o) noexcept -> decltype(std::declval<T&>().swap(o))
		{
			static_cast<T&>(*this).swap(o);
		}
	};
	template<typename T>
	constexpr auto swap(properly_noexcept_wrapper<T>& a,properly_noexcept_wrapper<T>& b) noexcept -> decltype(swap(static_cast<T&>(a),static_cast<T&>(b)))
	{
		return swap(static_cast<T&>(a),static_cast<T&>(b));
	}

	template<typename T>
	constexpr auto swap(T& a,properly_noexcept_wrapper<T>& b) noexcept -> decltype(swap(a,static_cast<T&>(b)))
	{
		return swap(a,static_cast<T&>(b));
	}

	template<typename T>
	constexpr auto swap(properly_noexcept_wrapper<T>& a,T& b) noexcept -> decltype(swap(static_cast<T&>(a),b))
	{
		return swap(static_cast<T&>(a),b);
	}

	template<typename T>
	using get_properly_noexcept=
		std::conditional<
			std::is_nothrow_default_constructible<T>::value&&
			std::is_nothrow_destructible<T>::value&&
			std::is_nothrow_move_assignable<T>::value&&
			std::is_nothrow_move_constructible<T>::value&&
			std::is_nothrow_swappable<T>::value,
			T,
			properly_noexcept_wrapper<T>
		>;

	template<typename T>
	using get_properly_noexcept_t=typename get_properly_noexcept<T>::type;

	template<typename T>
	class properly_noexcept_holder:public T {
	public:
		T value;
		constexpr properly_noexcept_holder() noexcept
		{}
		template<typename... Args>
		constexpr properly_noexcept_holder(Args&&... args) noexcept(std::is_nothrow_constructible<T,Args&& ...>::value):value(std::forward<Args>(args)...)
		{}
		constexpr properly_noexcept_holder(properly_noexcept_holder const&)=default;
		constexpr properly_noexcept_holder& operator=(properly_noexcept_holder const&)=default;
		constexpr properly_noexcept_holder(properly_noexcept_holder&& o) noexcept:T(static_cast<T&&>(o))
		{}
		constexpr properly_noexcept_holder& operator=(properly_noexcept_holder&& o) noexcept
		{
			value=std::move(o.value);
		}

		template<typename OTHER>
		constexpr auto swap(OTHER& o) noexcept -> decltype(std::declval<T&>().swap(o))
		{
			return value.swap(o);
		}

		template<typename OTHER>
		constexpr auto swap(properly_noexcept_holder<OTHER>& o) noexcept -> decltype(std::declval<T&>().swap(o.value))
		{
			return value.swap(o.value);
		}

		~properly_noexcept_holder() noexcept
		{}
	};

	template<typename T>
	constexpr auto swap(properly_noexcept_holder<T>& a,properly_noexcept_holder<T>& b) noexcept -> decltype(swap(a.value,b.value))
	{
		return swap(a.value,b.value);
	}

	template<typename T>
	constexpr auto swap(T& a,properly_noexcept_holder<T>& b) noexcept -> decltype(swap(a,b.value))
	{
		return swap(a,b.value);
	}
	template<typename T>
	constexpr auto swap(properly_noexcept_holder<T>& a,T& b) noexcept -> decltype(swap(a.value,b))
	{
		return swap(a.value,b);
	}
}
#endif