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
#pragma once
#ifndef EXITERATOR_H
#define EXITERATOR_H
#include <iterator>
#include "exretype.h"
#include <assert.h>
#ifdef _MSVC_LANG
#define _EXITERATOR_HAS_CPP20 (_MSVC_LANG>=202000l)
#define _EXITERATOR_HAS_CPP17 (_MSVC_LANG>=201700l)
#else
#define _EXITERATOR_HAS_CPP20 (__cplusplus>=202000l)
#define _EXITERATOR_HAS_CPP17 (__cplusplus>=201700l)
#endif
#if _EXITERATOR_HAS_CPP17
#define _EXITERATOR_NODISCARD [[nodiscard]]
#else
#define _EXITERATOR_NODISCARD
#endif
namespace exlib {

	namespace cstring_iterator_impl {

		/*
			Iterator for a c-string so that an end iterator can be passed to stl algorithms without calculating strlen.
		*/
		template<typename CharType,CharType terminator=CharType{}>
		class cstring_iterator {
			CharType* _loc;
		public:
			using reference=CharType&;
			using value_type=CharType;
			using pointer=CharType*;
			using iterator_category=std::forward_iterator_tag;
			using difference_type=std::ptrdiff_t;
			constexpr cstring_iterator(CharType* str=nullptr):_loc(str)
			{}
			template<typename Other>
			constexpr cstring_iterator(cstring_iterator<Other,terminator> str):_loc(str.operator->())
			{}

			_EXITERATOR_NODISCARD constexpr bool operator==(cstring_iterator o) const
			{
				return _loc==o._loc;
			}
			_EXITERATOR_NODISCARD constexpr bool operator!=(cstring_iterator o) const
			{
				return _loc!=o._loc;
			}
			constexpr cstring_iterator& operator++()
			{
				++_loc;
				if (*_loc==terminator)
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
			_EXITERATOR_NODISCARD constexpr reference operator*() const
			{
				return *_loc;
			}
			_EXITERATOR_NODISCARD constexpr pointer operator->() const
			{
				return _loc;
			}
			_EXITERATOR_NODISCARD explicit constexpr operator CharType* () const
			{
				return _loc;
			}
			_EXITERATOR_NODISCARD static constexpr cstring_iterator end()
			{
				return {nullptr};
			}
			_EXITERATOR_NODISCARD constexpr cstring_iterator begin() const
			{
				return *this;
			}
		};

		template<typename Iter>
		_EXITERATOR_NODISCARD constexpr Iter begin(Iter it)
		{
			return it;
		}
		template<typename Iter>
		_EXITERATOR_NODISCARD constexpr Iter end(Iter)
		{
			return {nullptr};
		}

#if _EXITERATOR_HAS_CPP17&&defined(__INTELLISENSE__)
		template<typename CharType>
		cstring_iterator(CharType*)->cstring_iterator<CharType>;
#endif
	}
	using cstring_iterator_impl::cstring_iterator;

	template<typename CharType>
	constexpr cstring_iterator<CharType> make_cstring_iterator(CharType* str)
	{
		return {str};
	}

	namespace iterator_detail {
		template<typename T>
		class has_decrement_operator {
			template<typename U>
			constexpr static auto get(U u)-> decltype(--u,u--,true);
			template<typename U,typename... Extra>
			constexpr static char get(U,Extra...);
		public:
			static constexpr bool value=std::is_same<bool,decltype(get(std::declval<T>()))>::value;
		};

		template<typename T,typename Derived,bool has_dec=has_decrement_operator<T>::value>
		struct inherit_decrement {
		private:
			Derived& get_chained() noexcept
			{
				return static_cast<Derived&>(*this);
			}
		public:
			constexpr Derived& operator--() noexcept(noexcept(--get_chained()._base))
			{
				Derived& me=get_chained();
				--me._base;
				return me;
			}
			constexpr Derived operator--(int) noexcept(noexcept(get_chained()._base--))
			{
				Derived& me=static_cast<Derived&>(*this);
				Derived copy(me);
				--me._base;
				return copy;
			}
		};

		template<typename T,typename Derived>
		struct inherit_decrement<T,Derived,false> {};

		template<typename T,typename PtrDiffT>
		class has_subscript_operator {
			template<typename U>
			constexpr static auto get(U u)-> decltype(u[PtrDiffT{}],true);
			template<typename U,typename... Extra>
			constexpr static char get(U,Extra...);
		public:
			static constexpr bool value=std::is_same<bool,decltype(get(std::declval<T>()))>::value;
		};

		template<typename T,typename PtrDiffT,typename Derived,typename ValueType,bool has_dec=has_subscript_operator<T,PtrDiffT>::value>
		struct inherit_subscript {
		private:
			constexpr Derived const& get_chained() const noexcept
			{
				return static_cast<Derived const&>(*this);
			}
		public:
			constexpr ValueType operator[](PtrDiffT s) const noexcept(noexcept(get_chained().functor()(get_chained()._base[s])))
			{
				return get_chained().functor()(get_chained()._base[s]);
			}
		};

		template<typename T,typename PtrDiffT,typename Derived,typename ValueType>
		struct inherit_subscript<T,PtrDiffT,Derived,ValueType,false> {};


		template<typename T,typename PtrDiffT>
		class has_plus_equal {
			template<typename U>
			constexpr static auto get(U u,PtrDiffT p)-> decltype(u+=p,true);
			template<typename U,typename... Extra>
			constexpr static char get(U,PtrDiffT p,Extra...);
		public:
			static constexpr bool value=std::is_same<bool,decltype(get(std::declval<T>(),std::declval<PtrDiffT>()))>::value;
		};

		template<typename T,typename PtrDiffT,typename Derived,bool has_dec=has_plus_equal<T,PtrDiffT>::value>
		struct inherit_plus_equal {
		private:
			constexpr Derived& get_chained() noexcept
			{
				return static_cast<Derived&>(*this);
			}
		public:
			constexpr Derived& operator+=(PtrDiffT p) noexcept(noexcept(get_chained()._base+=p))
			{
				Derived& me=static_cast<Derived&>(*this);
				me._base+=p;
				return me;
			}
		};

		template<typename T,typename PtrDiffT,typename Derived>
		struct inherit_plus_equal<T,PtrDiffT,Derived,false> {};

		template<typename T,typename PtrDiffT>
		class has_minus_equal {
			template<typename U>
			constexpr static auto get(U u,PtrDiffT p)-> decltype(u-=p,true);
			template<typename U,typename... Extra>
			constexpr static char get(U,PtrDiffT p,Extra...);
		public:
			static constexpr bool value=std::is_same<bool,decltype(get(std::declval<T>(),std::declval<PtrDiffT>()))>::value;
		};

		template<typename T,typename PtrDiffT,typename Derived,bool has_dec=has_minus_equal<T,PtrDiffT>::value>
		struct inherit_minus_equal {
		private:
			constexpr Derived& get_chained() noexcept
			{
				return static_cast<Derived&>(*this);
			}
		public:
			constexpr Derived& operator-=(PtrDiffT p) noexcept(noexcept(get_chained()._base-=p))
			{
				Derived& me=static_cast<Derived&>(*this);
				me._base-=p;
				return me;
			}
		};

		template<typename T,typename PtrDiffT,typename Derived>
		struct inherit_minus_equal<T,PtrDiffT,Derived,false> {};

		template<typename T>
		class ptr_wrapper {
			T _val;
		public:
			template<typename... Args>
			constexpr ptr_wrapper(Args&& ... args) noexcept(noexcept(T(std::forward<Args>(args)...))):_val(std::forward<Args>(args)...)
			{}
			constexpr T& operator*() const noexcept
			{
				return _val;
			}
			constexpr T* operator->() const noexcept
			{
				return &_val;
			}
		};

		template<typename T>
		using iter_diff_t=typename std::iterator_traits<T>::difference_type;

		template<typename Iter,typename Functor>
		using func_value_t=decltype(std::declval<Functor const&>()(std::declval<typename std::iterator_traits<Iter>::value_type>()));
	}

	template<typename Base,typename Functor>
	class transform_iterator:
		empty_store<Functor>,
		public iterator_detail::inherit_decrement<Base,transform_iterator<Base,Functor>>,
		public iterator_detail::inherit_plus_equal<Base,iterator_detail::iter_diff_t<Base>,transform_iterator<Base,Functor>>,
		public iterator_detail::inherit_minus_equal<Base,iterator_detail::iter_diff_t<Base>,transform_iterator<Base,Functor>>,
		public iterator_detail::inherit_subscript<Base,iterator_detail::iter_diff_t<Base>,transform_iterator<Base,Functor>,iterator_detail::func_value_t<Base,Functor>> {
		Base _base;
		using FunctorStore=empty_store<Functor>;
		friend struct iterator_detail::inherit_decrement<Base,transform_iterator<Base,Functor>>;
		friend struct iterator_detail::inherit_plus_equal<Base,iterator_detail::iter_diff_t<Base>,transform_iterator<Base,Functor>>;
		friend struct iterator_detail::inherit_minus_equal<Base,iterator_detail::iter_diff_t<Base>,transform_iterator<Base,Functor>>;
		friend struct iterator_detail::inherit_subscript<Base,iterator_detail::iter_diff_t<Base>,transform_iterator<Base,Functor>,iterator_detail::func_value_t<Base,Functor>>;
	public:
		using iterator_category=typename std::iterator_traits<Base>::iterator_category;
		using difference_type=iterator_detail::iter_diff_t<Base>;
		using value_type=iterator_detail::func_value_t<Base,Functor>;
		using reference=value_type;
		using pointer=iterator_detail::ptr_wrapper<value_type>;
		constexpr transform_iterator(Base base,Functor f) noexcept(std::is_nothrow_copy_constructible<Base>::value&& std::is_nothrow_move_constructible<Functor>::value):_base(base),FunctorStore(std::move(f))
		{}
		constexpr transform_iterator(Base base) noexcept(std::is_nothrow_copy_constructible<Base>::value&& std::is_nothrow_default_constructible<Functor>::value):_base(base)
		{}
		constexpr transform_iterator() noexcept(std::is_nothrow_default_constructible<Base>::value&& std::is_nothrow_default_constructible<Functor>::value)
		{}
		constexpr Base base() const noexcept(std::is_nothrow_copy_constructible<Base>::value)
		{
			return _base;
		}
		constexpr Functor functor() const noexcept(std::is_nothrow_copy_constructible<Functor>::value)
		{
			return empty_store<Functor>::get();
		}
		constexpr transform_iterator& operator++() noexcept(noexcept(++_base))
		{
			++_base;
			return *this;
		}
		constexpr transform_iterator operator++(int) noexcept(noexcept(++_base)&&std::is_nothrow_copy_constructible<transform_iterator>::value)
		{
			auto copy(*this);
			++_base;
			return copy;
		}
		constexpr reference operator*() const noexcept(noexcept(std::declval<Functor const&>()(*_base)))
		{
			return FunctorStore::get()(*_base);
		}
		constexpr pointer operator->() const noexcept(noexcept(std::declval<Functor const&>()(*_base)))
		{
			return {operator*()};
		}
	};

#define make_comp_op_for_gi(op) template<typename Base,typename Functor> constexpr auto operator op(transform_iterator<Base,Functor> const& a,transform_iterator<Base,Functor> const& b) noexcept(noexcept(a.base() op b.base())) -> decltype(a.base() op b.base()) {return a.base() op b.base();}
	make_comp_op_for_gi(==)
		make_comp_op_for_gi(!=)
		make_comp_op_for_gi(<)
		make_comp_op_for_gi(>)
		make_comp_op_for_gi(<=)
		make_comp_op_for_gi(>=)
#if _EXITERATOR_HAS_CPP20
		template<typename Base,typename Functor>
	constexpr auto operator<=>(transform_iterator<Base,Functor> const& a,transform_iterator<Base,Functor> const& b) noexcept(a.base()<=>b.base()) -> decltype(a.base()<=>b.base())
	{
		return a.base()<=>b.base();
	}
#endif
#undef make_comp_op_for_gi

	template<typename Base,typename Functor>
	constexpr auto operator+(transform_iterator<Base,Functor> const& it,typename transform_iterator<Base,Functor>::difference_type d) noexcept(noexcept(it.base()+d)&&std::is_nothrow_copy_constructible<Functor>::value) -> decltype(it.base()+d,transform_iterator<Base,Functor>{it.base(),it.functor()})
	{
		return {it.base()+d,it.functor()};
	}

	template<typename PtrDiffT,typename Base,typename Functor>
	constexpr auto operator+(PtrDiffT d,transform_iterator<Base,Functor> const& it) noexcept(noexcept(it+d)) -> decltype(it+d)
	{
		return it+d;
	}

	template<typename Base,typename Functor>
	constexpr auto operator-(transform_iterator<Base,Functor> const& it,typename transform_iterator<Base,Functor>::difference_type d) noexcept(noexcept(it.base()-d)&&std::is_nothrow_copy_constructible<Functor>::value) -> decltype(it.base()-d,transform_iterator<Base,Functor>{it.base(),it.functor()})
	{
		return {it.base()-d,it.functor()};
	}

	template<typename Base,typename Functor>
	constexpr auto operator-(transform_iterator<Base,Functor> const& a,transform_iterator<Base,Functor> const& b) noexcept(noexcept(a.base()-b.base())) -> decltype(a.base()-b.base(),typename transform_iterator<Base,Functor>::difference_type{})
	{
		return a.base()-b.base();
	}

	template<typename Base,typename Functor>
	constexpr transform_iterator<Base,typename std::decay<Functor>::type> make_transform_iterator(Base base,Functor&& f)
	{
		return transform_iterator<Base,typename std::decay<Functor>::type>(base,std::forward<Functor>(f));
	}

	template<typename IntegralType,IntegralType Increment=1>
	class count_iterator {
		IntegralType _index;
	public:
		constexpr count_iterator() noexcept
		{}
		constexpr count_iterator(IntegralType index) noexcept:_index(index)
		{}
		using value_type=IntegralType;
		using reference=value_type const&;
		using pointer=value_type const*;
		using difference_type=std::ptrdiff_t;
		using iterator_category=std::random_access_iterator_tag;
		constexpr value_type operator*() const noexcept
		{
			return _index;
		}
		constexpr pointer operator->() const noexcept
		{
			return &_index;
		}
		constexpr count_iterator operator++(int) noexcept
		{
			auto copy{*this};
			_index+=Increment;
			return copy;
		}
		constexpr count_iterator& operator++() noexcept
		{
			_index+=Increment;
			return *this;
		}
		constexpr count_iterator operator--(int) noexcept
		{
			auto copy{*this};
			_index-=Increment;
			return copy;
		}
		constexpr count_iterator& operator--() noexcept
		{
			_index-=Increment;
			return *this;
		}
		constexpr count_iterator& operator+=(difference_type dif) noexcept
		{
			_index+=dif*Increment;
			return *this;
		}
		constexpr count_iterator& operator-=(difference_type dif) noexcept
		{
			_index-=dif*Increment;
			return *this;
		}
		constexpr value_type operator[](difference_type s) const noexcept
		{
			return _index+s*Increment;
		}
	};
	template<typename IntegralType,IntegralType Increment>
	constexpr count_iterator<IntegralType,Increment> operator+(count_iterator<IntegralType,Increment> it,std::ptrdiff_t diff) noexcept
	{
		return {*it+diff*Increment};
	}
	template<typename IntegralType,IntegralType Increment>
	constexpr count_iterator<IntegralType,Increment> operator+(std::ptrdiff_t diff,count_iterator<IntegralType,Increment> it) noexcept
	{
		return {*it+diff*Increment};
	}
	template<typename IntegralType,IntegralType Increment>
	constexpr count_iterator<IntegralType,Increment> operator-(count_iterator<IntegralType,Increment> it,std::ptrdiff_t diff) noexcept
	{
		return {*it-diff*Increment};
	}
	template<typename IntegralType,IntegralType Increment>
	constexpr std::ptrdiff_t operator-(count_iterator<IntegralType,Increment> it,count_iterator<IntegralType,Increment> it2)
	{
		assert((*it-*it2)%Increment==0);
		return (*it-*it2)/Increment;
	}

#define make_comp_op_for_gi(op) template<typename Integral,Integral Increment> constexpr bool operator op(count_iterator<Integral,Increment> a,count_iterator<Integral,Increment> b) noexcept {return *a<*b;}
	make_comp_op_for_gi(==)
		make_comp_op_for_gi(!=)
		make_comp_op_for_gi(<)
		make_comp_op_for_gi(>)
		make_comp_op_for_gi(<=)
		make_comp_op_for_gi(>=)
#if _EXITERATOR_HAS_CPP20
		template<typename Integral,Integral Increment>
	constexpr auto operator<=>(count_iterator<Integral,Increment> a,count_iterator<Integral,Increment> b) noexcept
	{
		return *a<=>*b;
	}
#endif
#undef make_comp_op_for_gi

	using index_iterator=count_iterator<std::size_t>;

	template<typename IntegralType>
	count_iterator<IntegralType> make_count_iterator(IntegralType c)
	{
		return {c};
	}

	namespace filter_iterator_detail {
		struct get_arrow_operator {
			template<typename Iter>
			constexpr static auto get(Iter iter)-> decltype(iter.operator->())
			{
				return iter.operator->();
			}
			template<typename Type,typename... Extra>
			constexpr static Type* get(Type* ptr,Extra...)
			{
				return ptr;
			}
		};
	}
	template<typename Base,typename Functor>
	class filter_iterator:empty_store<Functor> {
		Base _base;
		Base _end;
		using Traits=std::iterator_traits<Base>;
		void advance_until_satisfied()
		{
			for (;_base!=_end;++_base)
			{
				if (empty_store<Functor>::get()(*_base))
				{
					return;
				}
			}
		}

	public:
		using value_type=typename Traits::value_type;
		using difference_type=typename Traits::difference_type;
		using pointer=typename Traits::pointer;
		using reference=typename Traits::reference;
		using iterator_category=std::forward_iterator_tag;

		constexpr filter_iterator(Base iter,Base end,Functor f={}) noexcept(std::is_nothrow_move_constructible<Functor>::value):_base(iter),_end(end),empty_store(std::move(f))
		{
			advance_until_satisfied();
		}

		constexpr Functor predicate() const noexcept(std::is_nothrow_copy_constructible<Functor>::value)
		{
			return empty_store<Functor>::get();
		}
		constexpr reference operator*() const noexcept
		{
			return *_base;
		}
		constexpr pointer operator->() const noexcept
		{
			return filter_iterator_detail::get_arrow_operator::get(_base);
		}
		constexpr filter_iterator& operator++() noexcept (noexcept(empty_store<Functor>::get()(*_base)))
		{
			++_base;
			advance_until_satisfied();
			return *this;
		}
		constexpr filter_iterator operator++(int) noexcept (noexcept(empty_store<Functor>::get()(*_base)))
		{
			auto copy(*this);
			++_base;
			advance_until_satisfied();
			return copy;
		}
		constexpr bool operator==(filter_iterator const& o) const noexcept
		{
			assert(_end==o._end);
			return _base==o._base;
		}
		constexpr bool operator!=(filter_iterator const& o) const noexcept
		{
			assert(_end==o._end);
			return _base!=o._base;
		}
		constexpr Base base() const noexcept(std::is_nothrow_copy_constructible<Base>::value)
		{
			return _base;
		}
	};

	template<typename Iter,typename Functor>
	filter_iterator<Iter,Functor> make_filter_iterator(Iter iter,Iter end,Functor f)
	{
		return {iter,end,std::move(f)};
	}

	template<typename... Iters>
	struct combined_iterator {
		std::tuple<Iters...> _iters;
	public:
		using value_type=std::tuple<typename std::iterator_traits<Iters>::value_type...>;
		using difference_type=typename std::common_type<typename std::iterator_traits<Iters>::difference_type...>::type;
		using reference=std::tuple<typename std::iterator_traits<Iters>::reference...>;
		using pointer=iterator_detail::ptr_wrapper<reference>;
		using iterator_category=typename std::common_type<typename std::iterator_traits<Iters>::iterator_category...>::type;
		constexpr combined_iterator(Iters... iters) noexcept:_iters(iters...)
		{
		}
		reference operator*() const noexcept
		{
		}
	};
}
#endif