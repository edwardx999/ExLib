/*
Copyright 2018 Edward Xie

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#ifndef EXALG_H
#define EXALG_H
#include <utility>
#include <array>
#include <functional>
#include <stddef.h>
#ifdef _MSVC_LANG
#define _EXALG_HAS_CPP_20 (_MSVC_LANG>=202000L)
#define _EXALG_HAS_CPP_17 (_MSVC_LANG>=201700L)
#define _EXALG_HAS_CPP_14 (_MSVC_LANG>=201400L)
#else
#define _EXALG_HAS_CPP_20 (__cplusplus>=202000L)
#define _EXALG_HAS_CPP_17 (__cplusplus>=201700L)
#define _EXALG_HAS_CPP_14 (__cplusplus>=201400L)
#endif
#if _EXALG_HAS_CPP_17
#define _EXALG_NODISCARD [[nodiscard]]
#else
#define _EXALG_NODISCARD
#endif
#include <exception>
#include <stddef.h>
#include <tuple>
#include "exretype.h"

#if _EXALG_HAS_CPP_17
namespace std {
	template<typename... Types>
	class variant;
}
#endif
namespace exlib {

	template<typename A,typename B,typename Compare=std::less<void>>
	_EXALG_NODISCARD constexpr typename max_cref<A,B>::type min(A&& a,B&& b,Compare c={}) noexcept(noexcept(c(a,b)))
	{
		if(c(a,b))
		{
			return std::forward<A>(a);
		}
		return std::forward<B>(b);
	}

	template<typename A,typename B,typename Compare=std::less<void>>
	_EXALG_NODISCARD constexpr typename max_cref<A,B>::type max(A&& a,B&& b,Compare c={}) noexcept (noexcept(c(a,b)))
	{
		if(c(a,b))
		{
			return std::forward<B>(b);
		}
		return std::forward<A>(a);
	}

	template<typename A,typename B,typename Compare=std::less<void>>
	_EXALG_NODISCARD constexpr std::pair<typename max_cref<A,B>::type,typename max_cref<A,B>::type> minmax(A&& a,B&& b,Compare c={}) noexcept(noexcept(c(a,b)))
	{
		if(c(a,b))
		{
			return {std::forward<A>(a),std::forward<B>(b)};
		}
		return {std::forward<B>(b),std::forward<A>(a)};
	}

	template<typename Iter,typename Transform,typename Compare=std::less<void>>
	_EXALG_NODISCARD constexpr Iter min_keyed_element(Iter begin,Iter end,Transform transform,Compare c={}) noexcept(noexcept(transform(*begin))&&noexcept(c(transform(*begin),transform(*begin))))
	{
		if(begin==end)
		{
			return end;
		}
		auto min_val=transform(*begin);
		auto min_iter=begin;
		++begin;
		for(;begin!=end;++begin)
		{
			auto value=transform(*begin);
			if(c(value,min_val))
			{
				min_iter=begin;
				min_val=std::move(value);
			}
		}
		return min_iter;
	}

	template<typename Iter,typename Transform,typename Compare=std::less<void>>
	_EXALG_NODISCARD constexpr Iter max_keyed_element(Iter begin,Iter end,Transform transform,Compare c={}) noexcept(noexcept(min_keyed_element(begin,end,std::move(transform),std::move(c))))
	{
		using type=decltype(transform(*begin));
		struct inverter {
			Compare c;
			constexpr bool operator()(type const& a,type const& b) const noexcept(noexcept(c(a,b)))
			{
				return !(c(a,b));
			}
		};
		return min_keyed_element(begin,end,std::move(transform),inverter{std::move(c)});
	}

	template<typename Iter,typename Transform,typename Compare=std::less<void>>
	_EXALG_NODISCARD constexpr std::pair<Iter,Iter> minmax_keyed_element(Iter begin,Iter end,Transform transform,Compare c={}) noexcept(noexcept(transform(*begin))&&noexcept(c(transform(*begin),transform(*begin))))
	{
		if(begin==end)
		{
			return end;
		}
		auto min_val=transform(*begin);
		auto max_val=min_val;
		auto min_iter=begin;
		auto max_iter=begin;
		++begin;
		for(;begin!=end;++begin)
		{
			auto value=transform(*begin);
			if(c(value,min_val))
			{
				min_iter=begin;
				min_val=std::move(value);
			}
			else
			{
				max_iter=begin;
				max_val=std::move(value);
			}
		}
		return {min_iter,max_iter};
	}
}


namespace get_impl {
#if !(_EXALG_HAS_CPP_20)
	using std::get;
#endif
	template<size_t I,typename Container>
	constexpr auto do_get(Container&& container) noexcept(noexcept(get<I>(container))) -> decltype(exlib::forward_like<Container&&>(get<I>(container)))
	{
		return exlib::forward_like<Container&&>(get<I>(container));
	}
	template<size_t I,typename Container,typename... Extra>
	constexpr auto do_get(Container&& container,Extra...) noexcept(noexcept(container[I])) -> decltype(exlib::forward_like<Container&&>(container[I]))
	{
		return exlib::forward_like<Container&&>(container[I]);
	}
	template<size_t I,typename T,size_t N>
	constexpr T&& do_get(T(&& arr)[N]) noexcept
	{
		return std::move(arr[I]);
	}
}

namespace exlib {

	template<size_t I,typename Container>
	constexpr auto get(Container&& cont) noexcept(noexcept(get_impl::do_get<I>(std::forward<Container>(cont)))) -> decltype(get_impl::do_get<I>(std::forward<Container>(cont)))
	{
		return get_impl::do_get<I>(std::forward<Container>(cont));
	}


	/*
		Call the member function as if it is a global function.
	*/
	template<typename T,typename Ret,typename... Args>
	constexpr Ret apply_mem_fn(T* obj,Ret(T::* mem_fn)(Args...),Args&& ... args)
	{
		return (obj->*mem_fn)(std::forward<Args>(args)...);
	}
#if	_EXALG_HAS_CPP_17
	/*
		Version that has the member function constexpr bound to it,
		MemFn is a member function pointer of T
	*/
	template<auto MemFn,typename T,typename... Args>
	constexpr decltype(auto) apply_mem_fn(T* obj,Args&& ... args)
	{
		return (obj->*MemFn)(std::forward<Args>(args)...);
	}
#endif
}
namespace swap_detail {
	template<typename A> //try ADL swap
	constexpr auto try_swap2(A& a,A& b) -> decltype(swap(a,b),void())
	{
		swap(a,b);
	}
	template<typename A,typename... Extra> //default to temp move
	constexpr void try_swap2(A& a,A& b,Extra...)
	{
		auto temp(std::move(a));
		a=std::move(b);
		b=std::move(temp);
	}
	template<typename A> //try member swap
	constexpr auto try_swap1(A& a,A& b) -> decltype(a.swap(b),void())
	{
		a.swap(b);
	}
	template<typename A,typename... Extra> //failed member swap, try second round
	constexpr void try_swap1(A& a,A& b,Extra...)
	{
		try_swap2(a,b);
	}
}
namespace exlib {
	template<typename A>
	constexpr void swap(A& a,A& b)
	{
		swap_detail::try_swap1(a,b);
	}

	template<typename T=void>
	struct less;

	template<typename T>
	struct less {
		constexpr bool operator()(T const& a,T const& b) const
		{
			return a<b;
		}
	};

#if _EXALG_HAS_CPP_14

	namespace detail {
		struct for_each_in_tuple_h {
			template<typename Tpl,typename Func>
			constexpr static void apply(Tpl&& tpl,Func f,std::index_sequence<>)
			{

			}
			template<typename Tpl,typename Func,size_t I>
			constexpr static void apply(Tpl&& tpl,Func f,std::index_sequence<I>)
			{
				using std::get;
				f(get<I>(std::forward<Tpl>(tpl)));
			}
			template<typename Tpl,typename Func,size_t I,size_t... Rest>
			constexpr static void apply(Tpl&& tpl,Func f,std::index_sequence<I,Rest...>)
			{
				using std::get;
				f(get<I>(std::forward<Tpl>(tpl)));
				apply(std::forward<Tpl>(tpl),f,std::index_sequence<Rest...>{});
			}
		};
	}

	template<typename Tpl,typename Func>
	constexpr void for_each_in_tuple(Tpl&& tpl,Func&& f)
	{
		detail::for_each_in_tuple_h::apply(std::forward<Tpl>(tpl),std::forward<Func>(f),std::make_index_sequence<std::tuple_size<typename std::remove_reference<Tpl>::type>::value>{});
	}
#endif

	template<>
	struct less<char const*> {
		constexpr bool operator()(char const* a,char const* b) const
		{
			for(size_t i=0;;++i)
			{
				if(a[i]<b[i])
				{
					return true;
				}
				if(b[i]<a[i])
				{
					return false;
				}
				if(a[i]==0)
				{
					return false;
				}
			}
		}
	};

	template<>
	struct less<char*>:public less<char const*> {};

	template<>
	struct less<void>:public less<char const*> {
		using less<char const*>::operator();
		template<typename A,typename B>
		constexpr auto operator()(A const& a,B const& b) const -> typename std::enable_if<!std::is_convertible<A,char const*>::value||!std::is_convertible<B,char const*>::value,bool>::type
		{
			return a<b;
		}
	};

	//comp is two-way "less-than" operator
	template<typename TwoWayIter,typename Comp>
	constexpr void qsort(TwoWayIter begin,TwoWayIter end,Comp comp)
	{
		if(begin!=end)
		{
			auto pivot=begin;
			auto it=begin;
			++it;
			for(;it!=end;++it)
			{
				if(comp(*it,*begin))
				{
					swap(*it,*(++pivot));
				}
			}
			swap(*begin,*pivot);
			qsort(begin,pivot,comp);
			qsort(pivot+1,end,comp);
		}
	}

	//sort by exlib::less
	template<typename iter>
	constexpr void qsort(iter begin,iter end)
	{
		using T=typename std::decay<decltype(*begin)>::type;
		qsort(begin,end,less<T>());
	}

	namespace detail {
		template<typename Iter,typename Comp>
		constexpr void sift_down(Iter begin,size_t parent,size_t dist,Comp c)
		{
			while(true)
			{
				size_t const child1=2*parent+1;
				if(child1<dist)
				{
					size_t const child2=child1+1;
					if(child2<dist)
					{
						size_t const max=c(begin[child1],begin[child2])?child2:child1;
						if(c(begin[parent],begin[max]))
						{
							swap(begin[parent],begin[max]);
							parent=max;
						}
						else
						{
							break;
						}
					}
					else
					{
						if(c(begin[parent],begin[child1]))
						{
							swap(begin[parent],begin[child1]);
							parent=child1;
						}
						else
						{
							break;
						}
					}
				}
				else
				{
					break;
				}
			}
		}

		template<typename Iter,typename Comp>
		constexpr void make_heap(Iter begin,size_t dist,Comp c)
		{
			for(size_t i=dist/2;i>0;)
			{
				--i;
				detail::sift_down(begin,i,dist,c);
			}
		}
	}

	template<typename Iter,typename Comp=std::less<typename std::iterator_traits<Iter>::value_type>>
	constexpr void make_heap(Iter begin,Iter end,Comp c={})
	{
		size_t const dist=end-begin;
		detail::make_heap(begin,dist,c);
	}

	template<typename Iter,typename Comp=std::less<typename std::iterator_traits<Iter>::value_type>>
	constexpr void pop_heap(Iter begin,Iter end,Comp c={})
	{
		size_t const dist=end-begin;
		if(dist==0)
		{
			return;
		}
		swap(*begin,*(end-1));
		detail::sift_down(begin,0,dist-1,c);
	}

	/*
		heapsort
		sorts between [begin,end) heapsort using c as a less-than comparison function
		@param begin random access iter pointing to beginning of range to sort
		@param end random access iter pointing 1 beyond valid range to sort
		@param c comparison function accepting type pointed to by Iter
	*/
	template<typename Iter,typename Comp=std::less<typename std::iterator_traits<Iter>::value_type>>
	constexpr void heapsort(Iter begin,Iter end,Comp c={})
	{
		size_t const n=end-begin;
		if(n<2)
		{
			return;
		}
		detail::make_heap(begin,n,c);
		for(size_t i=n;i>0;)
		{
			--i;
			//put max at end
			swap(begin[i],begin[0]);
			//sift new top down
			detail::sift_down(begin,0,i,c);
		}
	}

	//inserts the element AT elem into the range [begin,elem] according to comp assuming the range is sorted
	template<typename TwoWayIter,typename Comp>
	constexpr void insert_back(TwoWayIter const begin,TwoWayIter elem,Comp comp)
	{
		auto j=elem;
		while(j!=begin)
		{
			--j;
			if(comp(*elem,*j))
			{
				swap(*elem,*j);
				--elem;
			}
		}
	}

	template<typename TwoWayIter>
	constexpr void insert_back(TwoWayIter const begin,TwoWayIter elem)
	{
		using T=typename std::decay<decltype(*begin)>::type;
		insert_back(begin,elem,less<T>());
	}

	//comp is two-way "less-than" operator
	template<typename TwoWayIter,typename Comp>
	constexpr void isort(TwoWayIter const begin,TwoWayIter end,Comp comp)
	{
		if(begin==end) return;
		auto i=begin;
		++i;
		for(;i!=end;++i)
		{
			insert_back(begin,i,comp);
		}
	}

	//sort by exlib::less
	template<typename TwoWayIter>
	constexpr void isort(TwoWayIter begin,TwoWayIter end)
	{
		using T=typename std::decay<decltype(*begin)>::type;
		isort(begin,end,less<T>());
	}

	//comp is two-way "less-than" operator
	template<typename T,size_t N,typename Comp>
	constexpr std::array<T,N> sorted(std::array<T,N> const& arr,Comp c)
	{
		auto sorted=arr;
		if constexpr(N<10) isort(sorted.begin(),sorted.end(),c);
		else qsort(sorted.begin(),sorted.end(),c);
		return sorted;
	}

	template<typename T,size_t N>
	constexpr std::array<T,N> sorted(std::array<T,N> const& arr)
	{
		return sorted(arr,less<T>());
	}

	template<typename T>
	struct array_size:array_size<typename std::remove_cv<typename std::remove_reference<T>::type>::type> {};

	template<typename T,size_t N>
	struct array_size<std::array<T,N>>:std::integral_constant<size_t,N> {};

	template<typename T,size_t N>
	struct array_size<T[N]>:std::integral_constant<size_t,N> {};

#if _EXALG_HAS_CPP_17
	template<typename T>
	inline constexpr size_t array_size_v=array_size<T>::value;
#endif

	namespace detail {
		template<typename Arr1,typename Arr2,size_t... I,size_t... J>
		constexpr auto concat(Arr1 const& a,Arr2 const& b,std::index_sequence<I...>,std::index_sequence<J...>)
		{
			using T=std::remove_cv_t<std::remove_reference_t<decltype(a[0])>>;
			using U=std::remove_cv_t<std::remove_reference_t<decltype(b[0])>>;
			std::array<typename std::common_type<T,U>::type,sizeof...(I)+sizeof...(J)> ret{{a[I]...,b[J]...}};
			return ret;
		}
		template<typename A,typename B>
		constexpr auto str_concat(A const& a,B const& b)
		{
			constexpr size_t N=array_size<A>::value;
			constexpr size_t M=array_size<B>::value;
			constexpr size_t Nf=N==0?0:N-1;
			constexpr size_t Mf=M==0?0:M-1;
			return concat(a,b,std::make_index_sequence<Nf>(),std::make_index_sequence<Mf>());
		}
		template<typename A,typename B,typename... C>
		constexpr auto str_concat(A const& a,B const& b,C const& ... c)
		{
			return str_concat(str_concat(a,b),c...);
		}
	}

	//concatenate arrays (std::array<T,N> or T[N]) and returns an std::array<T,CombinedLen> of the two
	template<typename A,typename B>
	constexpr auto concat(A const& a,B const& b)
	{
		constexpr size_t N=array_size<A>::value;
		constexpr size_t M=array_size<B>::value;
		return detail::concat(a,b,std::make_index_sequence<N>(),std::make_index_sequence<M>());
	}

	//concatenate arrays (std::array<T,N> or T[N]) and returns an std::array<T,CombinedLen> of the arrays
	template<typename A,typename B,typename... C>
	constexpr auto concat(A const& a,B const& b,C const& ... c)
	{
		return concat(concat(a,b),c...);
	}

	//concatenate str arrays (std::array<T,N> or T[N]) and returns an std::array<T,CombinedLen> of the arrays
	template<typename A,typename B,typename... C>
	constexpr auto str_concat(A const& a,B const& b,C const& ... c)
	{
		return concat(detail::str_concat(a,b,c...),"");
	}

	template<typename T=void>
	struct compare {
		constexpr int operator()(T const& a,T const& b) const
		{
			if(a<b)
			{
				return -1;
			}
			if(a==b)
			{
				return 0;
			}
			return 1;
		}
	};

	template<>
	struct compare<char const*> {
		constexpr int operator()(char const* a,char const* b) const
		{
			for(size_t i=0;;++i)
			{
				if(a[i]<b[i])
				{
					return -1;
				}
				if(b[i]<a[i])
				{
					return 1;
				}
				if(a[i]==0)
				{
					return 0;
				}
			}
		}
	};

	template<>
	struct compare<char*>:public compare<char const*> {};

	template<>
	struct compare<void>:public compare<char const*> {
		using compare<char const*>::operator();
		template<typename A,typename B>
		constexpr auto operator()(A const& a,B const& b) const -> typename std::enable_if<!std::is_convertible<A,char const*>::value||!std::is_convertible<B,char const*>::value,bool>::type
		{
			if(a<b) return -1;
			if(a==b) return 0;
			return 1;
		}
	};

	//returns the iterator it for which c(target,*it)==0
	//if this is not found, end is returned
	template<typename it,typename T,typename ThreeWayComp>
	constexpr it binary_find(it begin,it end,T const& target,ThreeWayComp c)
	{
		auto old_end=end;
		while(true)
		{
			it i=(end-begin)/2+begin;
			if(i>=end)
			{
				return old_end;
			}
			auto const res=c(target,*i);
			if(res==0)
			{
				return i;
			}
			if(res<0)
			{
				end=i;
			}
			else
			{
				begin=i+1;
			}
		}
	}

	template<typename it,typename T>
	constexpr it binary_find(it begin,it end,T const& target)
	{
		return binary_find(begin,end,target,compare<typename std::decay<decltype(*begin)>::type>());
	}

	//converts three way comparison into a less than comparison
	template<typename ThreeWayComp=compare<void>>
	struct lt_comp:private ThreeWayComp {
		template<typename A,typename B>
		constexpr bool operator()(A const& a,B const& b) const
		{
			return ThreeWayComp::operator()(a,b)<0;
		}
	};

	//converts three way comparison into a greater than comparison
	template<typename ThreeWayComp=compare<void>>
	struct gt_comp:private ThreeWayComp {
		template<typename A,typename B>
		constexpr bool operator()(A const& a,B const& b) const
		{
			return ThreeWayComp::operator()(a,b)>0;
		}
	};

	//converts three way comparison into a less than or equal to comparison
	template<typename ThreeWayComp=compare<void>>
	struct le_comp:private ThreeWayComp {
		template<typename A,typename B>
		constexpr bool operator()(A const& a,B const& b) const
		{
			return ThreeWayComp::operator()(a,b)<=0;
		}
	};

	//converts three way comparison into a greater than or equal to comparison
	template<typename ThreeWayComp=compare<void>>
	struct ge_comp:private ThreeWayComp {
		template<typename A,typename B>
		constexpr bool operator()(A const& a,B const& b) const
		{
			return ThreeWayComp::operator()(a,b)>=0;
		}
	};

	//converts three way comparison into equality comparison
	template<typename ThreeWayComp=compare<void>>
	struct eq_comp:private ThreeWayComp {
		template<typename A,typename B>
		constexpr bool operator()(A const& a,B const& b) const
		{
			return ThreeWayComp::operator()(a,b)==0;
		}
	};

	//converts three way comparison into inequality comparison
	template<typename ThreeWayComp=compare<void>>
	struct ne_comp:private ThreeWayComp {
		template<typename A,typename B>
		constexpr bool operator()(A const& a,B const& b) const
		{
			return ThreeWayComp::operator()(a,b)!=0;
		}
	};

	//inverts comparison
	template<typename Comp=compare<void>>
	struct inv_comp:private Comp {
		template<typename A,typename B>
		constexpr auto operator()(A const& a,B const& b) const
		{
			return Comp::operator()(b,a);
		}
	};

	template<typename T=void>
	using greater=inv_comp<less<T>>;

	//use this to initialize ct_map
	template<typename Key,typename Value>
	struct map_pair {
	private:
		Key _key;
		Value _value;
	public:
		using key_type=Key;
		using mapped_type=Value;
		template<typename A,typename B>
		constexpr map_pair(A&& a,B&& b):_key(std::forward<A>(a)),_value(std::forward<B>(b))
		{}
		constexpr Key const& key() const
		{
			return _key;
		}
		constexpr Value const& value() const
		{
			return _value;
		}
		constexpr Value& value()
		{
			return _value;
		}
	};

	namespace detail {
		template<typename Comp,typename Key,typename Value>
		struct map_compare:protected Comp {
			template<typename Conv>
			constexpr int operator()(Conv const& target,map_pair<Key,Value> const& b) const
			{
				return Comp::operator()(target,b.key());
			}
			constexpr int operator()(map_pair<Key,Value> const& a,map_pair<Key,Value> const& b) const
			{
				return Comp::operator()(a.key(),b.key());
			}
		};
	}

	//Comp defines operator()(Key(&),Key(&)) that is a three-way comparison
	template<typename Key,typename Value,size_t entries,typename Comp=compare<Key>>
	class ct_map:protected detail::map_compare<Comp,Key,Value>,protected std::array<map_pair<Key,Value>,entries> {
	public:
		using key_type=Key;
		using mapped_type=Value;
		using value_type=map_pair<Key,Value>;
	protected:
		using Data=std::array<value_type,entries>;
	public:
		using size_type=size_t;
		using difference_type=std::ptrdiff_t;
		using key_compare=detail::map_compare<Comp,Key,Value>;
		using typename Data::reference;
		using typename Data::const_reference;
		using typename Data::iterator;
		using typename Data::const_iterator;
		using typename Data::reverse_iterator;
		using typename Data::const_reverse_iterator;
	private:
		using pair=value_type;
	public:

		using Data::size;
		using Data::max_size;
		using Data::empty;
		using Data::operator[];

		using Data::begin;
		using Data::end;
		using Data::cbegin;
		using Data::cend;
		using Data::rbegin;
		using Data::rend;
		using Data::crbegin;
		using Data::crend;
		using Data::at;
		using Data::back;
		using Data::front;
		using Data::data;

		template<typename... Args>
		constexpr ct_map(Args&& ... rest):Data{{std::forward<Args>(rest)...}}
		{
			static_assert(sizeof...(Args)==entries,"Wrong number of entries");
			qsort(begin(),end(),lt_comp<key_compare>());
		}

	private:
		template<size_t... Is>
		constexpr ct_map(std::array<value_type,entries> const& in,std::index_sequence<Is...>):Data{{in[Is]...}}
		{}
	public:
		constexpr ct_map(std::array<value_type,entries> const& in):ct_map(in,std::make_index_sequence<entries>())
		{}
		template<typename T>
		constexpr iterator find(T const& k)
		{
			return binary_find(begin(),end(),k,static_cast<key_compare>(*this));
		}
		template<typename T>
		constexpr const_iterator find(T const& k) const
		{
			return binary_find(begin(),end(),k,static_cast<key_compare>(*this));
		}
	};

	//inputs should be of type map_pair<Key,Value>
	template<typename Comp,typename First,typename... Rest>
	constexpr auto make_ct_map(First&& f,Rest&& ... r)
	{
		return ct_map<typename First::key_type,typename First::mapped_type,1+sizeof...(r),Comp>(std::forward<First>(f),std::forward<Rest>(r)...);
	}

	//inputs should be of type map_pair<Key,Value>
	template<typename First,typename... T>
	constexpr auto make_ct_map(First&& k,T&& ... rest)
	{
		return make_ct_map<compare<typename First::key_type>>(std::forward<First>(k),std::forward<T>(rest)...);
	}

	template<typename Comp,typename T,size_t N>
	constexpr auto make_ct_map(std::array<T,N> const& in)
	{
		return ct_map<typename T::key_type,typename T::mapped_type,N,Comp>(in);
	}

	template<typename T,size_t N>
	constexpr auto make_ct_map(std::array<T,N> const& in)
	{
		return make_ct_map<compare<typename T::key_type>>(in);
	}

	namespace detail {
		template<typename B,typename... R>
		struct ma_ret {
			using type=B;
		};

		template<typename... R>
		struct ma_ret<void,R...> {
			using type=typename std::common_type<R...>::type;
		};
	}

	template<typename Type=void,typename... Args>
	constexpr std::array<typename detail::ma_ret<Type,Args...>::type,sizeof...(Args)> make_array(Args&& ... args)
	{
		return
		{{
			std::forward<Args>(args)...
		}};
	}

	namespace detail {
		template<typename Type,typename Tuple,typename Ix>
		struct ca_type_h;
		template<typename Type,typename Tuple,size_t... Ix>
		struct ca_type_h<Type,Tuple,std::index_sequence<Ix...>> {
			using type=std::array<typename ma_ret<Type,typename std::tuple_element<Ix,Tuple>::type...>::type,sizeof...(Ix)>;
		};
		template<typename Type,typename Tuple>
		struct ca_type:ca_type_h<Type,
			typename std::remove_reference<Tuple>::type,
			std::make_index_sequence<std::tuple_size<typename std::remove_reference<Tuple>::type>::value>>{

		};

		template<typename Type,typename Tuple,size_t... I>
		constexpr typename ca_type<Type,Tuple>::type conv_array(Tuple&& tup,std::index_sequence<I...>)
		{
			return {{  std::get<I>(std::forward<Tuple>(tup))... }};
		}
	}

	template<typename Type=void,typename Tuple>
	constexpr typename detail::ca_type<Type,Tuple>::type conv_array(Tuple&& args)
	{
		constexpr auto TS=std::tuple_size_v<typename std::remove_reference<Tuple>::type>;
		return detail::conv_array<Type>(std::forward<Tuple>(args),std::make_index_sequence<TS>());
	}

	//the number of elements accessible by std::get
	template<typename T>
	struct get_max {
	private:
		template<typename U>
		struct gm {
			constexpr static size_t const value=0;
		};

		template<typename... U>
		struct gm<std::tuple<U...>> {
			constexpr static size_t value=sizeof...(U);
		};

		template<typename U,size_t N>
		struct gm<std::array<U,N>> {
			constexpr static size_t value=N;
		};

#if _EXALG_HAS_CPP_17
		template<typename... U>
		struct gm<std::variant<U...>> {
			constexpr static size_t value=sizeof...(U);
		};
#endif

	public:
		constexpr static size_t const value=gm<std::remove_cv_t<std::remove_reference_t<T>>>::value;
	};

#if _EXALG_HAS_CPP_17
	template<typename T>
	constexpr size_t get_max_v=get_max<T>::value;

	namespace detail {

		template<typename Ret,size_t I,typename Funcs,typename...Args>
		constexpr Ret apply_single(Funcs&& funcs,Args&& ... args)
		{
			return static_cast<Ret>(std::get<I>(std::forward<Funcs>(funcs))(std::forward<Args>(args)...));
		}

		template<typename Ret,size_t... Is,typename Funcs,typename... Args>
		Ret apply_ind_jump_h(size_t i,std::index_sequence<Is...>,Funcs&& funcs,Args&& ... args)
		{
			using Func=Ret(Funcs&&,Args&&...);
			static constexpr Func* jtable[]={&apply_single<Ret,Is,Funcs,Args...>...};
			return jtable[i](std::forward<Funcs>(funcs),std::forward<Args>(args)...);
		}

		template<typename Ret,size_t N,typename Funcs,typename... Args>
		constexpr Ret apply_ind_jump(size_t i,Funcs&& funcs,Args&& ... args)
		{
			return apply_ind_jump_h<Ret>(i,std::make_index_sequence<N>(),std::forward<Funcs>(funcs),std::forward<Args>(args)...);
		}

		template<typename Ret,size_t I,size_t Max,typename Tuple,typename... Args>
		constexpr Ret apply_ind_linear_h(size_t i,Tuple&& funcs,Args&& ... args)
		{
			if constexpr(I<Max)
			{
				if(i==I)
				{
					return apply_single<Ret,I>(std::forward<Tuple>(funcs))(std::forward<Args>(args)...);
				}
				return apply_ind_linear_h<Ret,I+1,Max>(i,std::forward<Tuple>(funcs),std::forward<Args>(args)...);
			}
			else
			{
				throw std::invalid_argument("Index too high");
			}
		}

		template<typename Ret,size_t NumFuncs,typename Tuple,typename... Args>
		constexpr Ret apply_ind_linear(size_t i,Tuple&& funcs,Args&& ... args)
		{
			return apply_ind_linear_h<Ret,0,NumFuncs>(i,std::forward<Tuple>(funcs),std::forward<Args>(args)...);
		}

		template<typename Ret,size_t Lower,size_t Upper,typename Funcs,typename... Args>
		constexpr Ret apply_ind_bh(size_t i,Funcs&& funcs,Args&& ... args)
		{
			if constexpr(Lower<Upper)
			{
				constexpr size_t I=(Upper-Lower)/2+Lower;
				if(i==I)
				{
					return apply_single<Ret,I>(std::forward<Funcs>(funcs),std::forward<Args>(args)...);
				}
				else if(i<I)
				{
					return apply_ind_bh<Ret,Lower,I>(i,std::forward<Funcs>(funcs),std::forward<Args>(args)...);
				}
				else
				{
					return apply_ind_bh<Ret,I+1,Upper>(i,std::forward<Funcs>(funcs),std::forward<Args>(args)...);
				}
			}
			else
			{
				throw std::invalid_argument("Index too high");
			}
		}

		template<typename Ret,size_t NumFuncs,typename Funcs,typename... Args>
		constexpr Ret apply_ind_bsearch(size_t i,Funcs&& funcs,Args&& ... args)
		{
			return apply_ind_bh<Ret,0,NumFuncs>(i,std::forward<Funcs>(funcs),std::forward<Args>(args)...);
		}
	}

	//Returns static_cast<Ret>(std::get<i>(std::forward<Funcs>(funcs))(std::forward<Args>(args)...)); Ret can be void.
	//Assumes i is less than NumFuncs, otherwise behavior is undefined.
	//Other overloads automatically determine Ret and NumFuncs if they are not supplied.
	template<typename Ret,size_t NumFuncs,typename Funcs,typename... Args>
	constexpr decltype(auto) apply_ind(size_t i,Funcs&& funcs,Args&& ... args)
	{
		//MSVC currently can't inline the function pointers used by jump so I have a somewhat arbitrary
		//heuristic for choosing which apply to use
		if constexpr(NumFuncs<4)
		{
			return detail::apply_ind_bsearch<Ret,NumFuncs>(i,std::forward<Funcs>(funcs),std::forward<Args>(args)...);
		}
		else
		{
			return detail::apply_ind_jump<Ret,NumFuncs>(i,std::forward<Funcs>(funcs),std::forward<Args>(args)...);
		}
	}

	template<size_t NumFuncs,typename Ret,typename Funcs,typename... Args>
	constexpr decltype(auto) apply_ind(size_t i,Funcs&& funcs,Args&& ... args)
	{
		return apply_ind<Ret,NumFuncs>(i,std::forward<Funcs>(funcs),std::forward<Args>(args)...);
	}

	template<size_t NumFuncs,typename Funcs,typename... Args>
	constexpr decltype(auto) apply_ind(size_t i,Funcs&& funcs,Args&& ... args)
	{
		if constexpr(NumFuncs==0)
		{
			return;
		}
		else
		{
			using Ret=decltype(std::get<0>(std::forward<Funcs>(funcs))(std::forward<Args>(args)...));
			return apply_ind<Ret,NumFuncs>(i,std::forward<Funcs>(funcs),std::forward<Args>(args)...);
		}
	}

	template<typename Ret,typename Funcs,typename... Args>
	constexpr decltype(auto) apply_ind(size_t i,Funcs&& funcs,Args&& ... args)
	{
		constexpr size_t N=get_max<std::remove_cv_t<std::remove_reference_t<Funcs>>>::value;
		return apply_ind<Ret,N>(i,std::forward<Funcs>(funcs),std::forward<Args>(args)...);
	}

	template<typename Funcs,typename... Args>
	constexpr decltype(auto) apply_ind(size_t i,Funcs&& funcs,Args&& ... args)
	{
		constexpr size_t N=get_max<std::remove_cv_t<std::remove_reference_t<Funcs>>>::value;
		return apply_ind<N>(i,std::forward<Funcs>(funcs),std::forward<Args>(args)...);
	}
#endif
	template<typename FindNext,typename... IndParser>
	class CSVParserBase:protected FindNext,protected std::tuple<IndParser...> {
	private:
		using Parsers=std::tuple<IndParser...>;
	public:
		size_t parse(char const* str,size_t len)
		{
			throw "Not implemented";
		}
		size_t parse(char const*)
		{
			throw "Not implemented";
		}
	};
}
#endif