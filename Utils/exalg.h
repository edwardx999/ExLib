#ifndef EXALG_H
#define EXALG_H
#include <utility>
#include <array>
#include <functional>
namespace exlib {

	template<typename A>
	constexpr void swap(A& a,A& b)
	{
		auto temp=std::move(a);
		a=std::move(b);
		b=std::move(temp);
	}

	template<typename T>
	struct less:public std::less<T> {};

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

	//comp is two-way "less-than" operator
	template<typename iter,typename Comp>
	constexpr void qsort(iter begin,iter end,Comp comp)
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

	//comp is two-way "less-than" operator
	template<typename iter,typename Comp>
	constexpr void isort(iter begin,iter end,Comp comp)
	{
		for(auto it=begin;it!=end;++it)
		{
			for(auto j=begin;j!=it;++j)
			{
				if(comp(*it,*j))
				{
					swap(*it,*j);
				}
			}
		}
	}

	//sort by exlib::less
	template<typename iter>
	constexpr void isort(iter begin,iter end)
	{
		using T=typename std::decay<decltype(*begin)>::type;
		isort(begin,end,less<T>());
	}

	namespace detail {
		template<typename T,size_t N,typename Comp>
		constexpr void sort(std::array<T,N>& a,Comp c,size_t left,size_t right)
		{
			if(left<right)
			{
				size_t pivot=left;
				for(size_t i=left+1;i<right;++i)
				{
					if(c(a[i],a[left]))
					{
						swap(a[i],a[++pivot]);
					}
				}
				swap(a[left],a[pivot]);
				sort(a,c,left,pivot);
				sort(a,c,pivot+1,right);
			}
		}
	}

	//comp is two-way "less-than" operator
	template<typename T,size_t N,typename Comp>
	constexpr std::array<T,N> sorted(std::array<T,N> const& arr,Comp c)
	{
		auto sorted=arr;
		detail::sort(sorted,c,0,N);
		return sorted;
	}

	template<typename T,size_t N>
	constexpr std::array<T,N> sorted(std::array<T,N> const& arr)
	{
		return sorted(arr,less<T>());
	}

	namespace detail {
		template<typename T,size_t N,size_t M,size_t... I,size_t... J>
		constexpr auto concat(std::array<T,N> const& a,std::array<T,M> const& b,std::index_sequence<I...>,std::index_sequence<J...>)
		{
			std::array<T,N+M> ret{{a[I]...,b[J]...}};
			return ret;
		}
	}

	template<typename T,size_t N,size_t M>
	constexpr auto concat(std::array<T,N> const& a,std::array<T,M> const& b)
	{
		return detail::concat(a,b,std::make_index_sequence<N>(),std::make_index_sequence<M>());
	}

	template<typename T>
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

	template<typename it,typename T,typename ThreeWayComp>
	constexpr it binary_find(it begin,it end,T const& target,ThreeWayComp c)
	{
		auto old_end=end;
		while(true)
		{
			it i=(end-begin)/2+begin;
			if(i>=end)
			{
				break;
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
		return old_end;
	}

	template<typename it,typename T>
	constexpr it binary_find(it begin,it end,T const& target)
	{
		return binary_find(begin,end,target,compare<std::decay<decltype(*begin)>::type>());
	}

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
		constexpr Key const& key() const{
			return _key;
		}
		constexpr Value const& value() const
		{
			return value;
		}
		constexpr Value& value()
		{
			return value;
		}
	};

	namespace detail {
		template<typename Comp,typename Key,typename Value>
		struct map_compare:protected Comp {
			constexpr int operator()(Key const& target,map_pair<Key,Value> const& b) const
			{
				return Comp::operator()(target,b.key());
			}
			constexpr int operator()(map_pair<Key,Value> const& a,map_pair<Key,Value> const& b) const
			{
				return Comp::operator()(a.key(),b.key());
			}
		};
	}

	//converts three way comparison into a less than comparison
	template<typename ThreeWayComp>
	struct lt_comp:private ThreeWayComp {
		template<typename A,typename B>
		constexpr bool operator()(A const& a,B const& b) const
		{
			return ThreeWayComp::operator()(a,b)<0;
		}
	};

	//converts three way comparison into a greater than comparison
	template<typename ThreeWayComp>
	struct gt_comp:private ThreeWayComp {
		template<typename A,typename B>
		constexpr bool operator()(A const& a,B const& b) const
		{
			return ThreeWayComp::operator()(a,b)>0;
		}
	};

	//converts three way comparison into a less than or equal to comparison
	template<typename ThreeWayComp>
	struct le_comp:private ThreeWayComp {
		template<typename A,typename B>
		constexpr bool operator()(A const& a,B const& b) const
		{
			return ThreeWayComp::operator()(a,b)<=0;
		}
	};

	//converts three way comparison into a greater than comparison
	template<typename ThreeWayComp>
	struct ge_comp:private ThreeWayComp {
		template<typename A,typename B>
		constexpr bool operator()(A const& a,B const& b) const
		{
			return ThreeWayComp::operator()(a,b)>=0;
		}
	};

	//converts three way comparison into equality comparison
	template<typename ThreeWayComp>
	struct eq_comp:private ThreeWayComp {
		template<typename A,typename B>
		constexpr bool operator()(A const& a,B const& b) const
		{
			return ThreeWayComp::operator()(a,b)==0;
		}
	};

	//Comp defines operator()(Key(&),Key(&)) that returns <0, 0, or >0
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
		constexpr ct_map(Args&&... rest):Data{{std::forward<Args>(rest)...}}
		{
			static_assert(sizeof...(Args)==entries,"Wrong number of entries");
			qsort(begin(),end(),lt_comp<key_compare>());
		}

		constexpr iterator find(Key const& k)
		{
			return binary_find(begin(),end(),k,static_cast<key_compare>(*this));
		}
		constexpr const_iterator find(Key const& k) const
		{
			return binary_find(begin(),end(),k,static_cast<key_compare>(*this));
		}
	};


	template<typename Comp,typename First,typename... Rest>
	constexpr auto make_ct_map(First&& f,Rest&&... r)
	{
		return ct_map<First::key_type,First::mapped_type,1+sizeof...(r),Comp>(std::forward<First>(f),std::forward<Rest>(r)...);
	}

	template<typename First,typename... T>
	constexpr auto make_ct_map(First&& k,T&&... rest)
	{
		return make_ct_map<compare<First::key_type>>(std::forward<First>(k),std::forward<T>(rest)...);
	}

	template<typename FindNext,typename... IndParser>
	class CSVParserBase:protected FindNext,protected std::tuple<IndParser...> {

	public:
		size_t parse(std::string_view s)
		{

		}
	};
}
#endif