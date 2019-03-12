#pragma once
#ifndef EXITERATOR_H
#define EXITERATOR_H
#include <iterator>
#ifdef _MSVC_LANG
#define EXITERATOR_HAS_CPP17 _MSVC_LANG>=201700l
#else
#define EXITERATOR_HAS_CPP17 __cplusplus>=201700l
#endif
namespace exlib {

	namespace cstring_iterator_impl {
		/*
			Iterator for a c-string so that an end iterator can be passed to stl algorithms without calculating strlen.
		*/
		template<typename StrType>
		class cstring_iterator {
			StrType _loc;
		public:
			using reference=decltype(*_loc);
			using value_type=typename std::remove_cv<typename std::remove_reference<reference>::type>::type;
			using pointer=StrType;
			using iterator_category=std::forward_iterator_tag;
			using difference_type=std::ptrdiff_t;
			constexpr cstring_iterator(StrType str=nullptr):_loc(str)
			{}
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
				if(*_loc==value_type())
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
		};
		template<typename Base>
		constexpr cstring_iterator<Base> begin(cstring_iterator<Base> it)
		{
			return it;
		}
		template<typename Base>
		constexpr cstring_iterator<Base> end(cstring_iterator<Base>)
		{
			return {nullptr};
		}
	}
	using cstring_iterator_impl::cstring_iterator;
}
#endif