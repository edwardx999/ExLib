#pragma once
/*
Copyright 2019 Edward Xie

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#ifndef EXRANGE_H
#define EXRANGE_H
#include <iterator>
#include <type_traits>
namespace exlib {

	namespace range_detail {

		template<typename It, typename S>
		auto has_random_access(It&& it, S&& s) -> decltype(it - s, 0);

		template<typename It, typename S, typename... Extra>
		float has_random_access(It&& it, S&& s, Extra...);

		template<typename Derived, typename Iterator, typename Sentinal,
			bool random_access =
			std::is_same<int, decltype(has_random_access(std::declval<Iterator>(), std::declval<Sentinal>()))>::value>
		class inherit_size {
				constexpr Derived const& self() const noexcept
				{
					return static_cast<Derived const&>(*this);
				}
			public:
				constexpr typename std::make_unsigned<typename std::iterator_traits<Iterator>::difference_type>::type size() const noexcept
				{
					return self().end() - self().begin();
				}
				template<typename IndexType>
				constexpr typename std::iterator_traits<Iterator>::reference operator[](IndexType n) const noexcept
				{
					auto const bbegin = self().begin();
					return bbegin[n];
				}
		};

		template<typename Derived, typename Iterator, typename Sentinal>
		class inherit_size<Derived, Iterator, Sentinal, false> {};

		template<typename Derived, typename Iterator, typename Sentinal, bool is_pointer = std::is_pointer<Iterator>::value&& std::is_same<Iterator, Sentinal>::value>
		class inherit_data {
		public:
			constexpr Iterator data() const noexcept
			{
				return Derived::begin();
			}
		};

		template<typename Derived, typename Iterator, typename Sentinal>
		class inherit_data<Derived, Iterator, Sentinal, false> {};

		template<typename It, typename S>
		auto decrementable(It&& it, S&& s) -> decltype(--it, --s, 0);

		template<typename It, typename S, typename... Extra>
		float decrementable(It&& it, S&& s, Extra...);

		template<typename Derived, typename Iterator, typename Sentinal,
			bool is_decrementable =
			std::is_same<int, decltype(decrementable(std::declval<Iterator>(), std::declval<Sentinal>()))>::value&&
			std::is_same<Iterator, Sentinal>::value>
			class inherit_rbegin {
			public:
				constexpr std::reverse_iterator<Iterator> rbegin() const noexcept
				{
					return std::reverse_iterator<Iterator>(Derived::end());
				}
				constexpr std::reverse_iterator<Iterator> rend() const noexcept
				{
					return std::reverse_iterator<Iterator>(Derived::begin());
				}
		};

		template<typename Derived, typename Iterator, typename Sentinal>
		class inherit_rbegin<Derived, Iterator, Sentinal, false> {};
	}

	template<typename Iterator, typename Sentinal = Iterator>
	class pair_range:
		public range_detail::inherit_data<pair_range<Iterator, Sentinal>, Iterator, Sentinal>,
		public range_detail::inherit_size<pair_range<Iterator, Sentinal>, Iterator, Sentinal>,
		public range_detail::inherit_rbegin<pair_range<Iterator, Sentinal>, Iterator, Sentinal> {
		Iterator _begin;
		Sentinal _end;
	public:
		constexpr pair_range(Iterator begin, Sentinal end) noexcept:_begin(begin), _end(end)
		{}

		constexpr Iterator begin() const noexcept
		{
			return _begin;
		}

		constexpr Sentinal end() const noexcept
		{
			return _end;
		}
	};

	template<typename Iterator, typename Sentinal>
	constexpr pair_range<Iterator, Sentinal> make_pair_range(Iterator begin, Sentinal end) noexcept
	{
		return pair_range<Iterator, Sentinal>(begin, end);
	}
}

#endif