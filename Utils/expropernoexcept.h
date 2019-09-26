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
		~properly_noexcept_wrapper() noexcept{}

		template<typename OTHER>
		constexpr auto swap(OTHER& o) noexcept -> decltype(std::declval<T&>().swap(o))
		{
			static_cast<T&>(*this).swap(o);
		}
	};

	template<typename T>
	constexpr auto swap(properly_noexcept_wrapper<T>& a,properly_noexcept_wrapper<T>& b) noexcept -> decltype(swap(static_cast<T&>(a),static_cast<T&>(b)))
	{
		swap(static_cast<T&>(a),static_cast<T&>(b));
	}

	template<typename T>
	class properly_noexcept_holder:public T {
	public:
		T value;
		constexpr properly_noexcept_holder() noexcept
		{}
		template<typename... Args>
		constexpr properly_noexcept_holder(Args&&... args):value(std::forward<Args>(args)...)
		{}
		constexpr properly_noexcept_holder(properly_noexcept_holder&& o) noexcept:T(static_cast<T&&>(o))
		{}
		constexpr properly_noexcept_holder& operator=(properly_noexcept_holder&& o) noexcept
		{
			value=std::move(o.value);
		}

		template<typename OTHER>
		constexpr auto swap(OTHER& o) noexcept -> decltype(std::declval<T&>().swap(o))
		{
			static_cast<T&>(*this).swap(o);
		}

		~properly_noexcept_holder() noexcept
		{}
	};

	template<typename T>
	constexpr auto swap(properly_noexcept_holder<T>& a,properly_noexcept_holder<T>& b) noexcept -> decltype(swap(a.value,b.value))
	{
		swap(a.value,b.value);
	}
}
#endif