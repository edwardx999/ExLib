#pragma once
#ifndef EXITERATOR_H
#define EXITERATOR_H
#include <iterator>
#ifdef _MSVC_LANG
#define _EXITERATOR_HAS_CPP17 _MSVC_LANG>=201700l
#else
#define _EXITERATOR_HAS_CPP17 __cplusplus>=201700l
#endif
namespace exlib {

	namespace cstring_iterator_impl {

		template<typename StrType>
		using char_type_t=typename std::remove_cv<typename std::iterator_traits<StrType>::value_type>::type;

		/*
			Iterator for a c-string so that an end iterator can be passed to stl algorithms without calculating strlen.
		*/
		template<typename StrType,char_type_t<StrType> delim=char_type_t<StrType>{}>
		class cstring_iterator {
			StrType _loc;
		public:
			using reference=decltype(*_loc);
			using value_type=char_type_t<StrType>;
			using pointer=StrType;
			using iterator_category=std::forward_iterator_tag;
			using difference_type=std::ptrdiff_t;
			constexpr cstring_iterator(StrType str=nullptr):_loc(str)
			{}
			template<typename Other>
			constexpr cstring_iterator(cstring_iterator<Other,delim> str):_loc(static_cast<StrType>(str)){}

			constexpr bool operator==(cstring_iterator o) const
			{
				return _loc==o._loc;
			}
			constexpr bool operator!=(cstring_iterator o) const
			{
				return _loc!=o._loc;
			}
			constexpr cstring_iterator& operator++()
			{
				++_loc;
				if(*_loc==delim)
				{
					_loc=nullptr;
				}
				return *this;
			}
			constexpr cstring_iterator operator++(int)
			{
				auto copy(*this);
				++(*this);
				return copy;
			}
			constexpr reference operator*() const
			{
				return *_loc;
			}
			constexpr pointer operator->() const
			{
				return _loc;
			}
			explicit constexpr operator StrType&()
			{
				return _loc;
			}
			explicit constexpr operator StrType const&() const
			{
				return _loc;
			}
		};

		template<typename Base,char_type_t<Base> delim>
		constexpr cstring_iterator<Base,delim> begin(cstring_iterator<Base,delim> it)
		{
			return it;
		}
		template<typename Base,char_type_t<Base> delim>
		constexpr cstring_iterator<Base> end(cstring_iterator<Base,delim>)
		{
			return {nullptr};
		}
	}
	using cstring_iterator_impl::cstring_iterator;

	template<typename StrType>
	constexpr cstring_iterator<StrType> make_cstring_iterator(StrType str)
	{
		return {str};
	}
}
#endif