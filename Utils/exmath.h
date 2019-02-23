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
#ifndef EXMATH_H
#define EXMATH_H
#include <type_traits>
#include <vector>
#include <functional>
#include <algorithm>
#include <array>
#include <cstdlib>
#include <assert.h>
#if defined(_CONSTEXPR17)
#define EX_CONSTEXPR _CONSTEXPR17
#elif defined(_HAS_CXX17) || __cplusplus>201100L
#define EX_CONSTEXPR constexpr
#else
#define EX_CONSTEXPR
#endif

#define CONSTEXPR
#ifdef _CONSTEXPR_IF
#define EX_CONSTIF _CONSTEXPR_IF
#elif defined(_HAS_CXX17) || __cplusplus>201700L
#define EX_CONSTIF constexpr
#else
#define EX_CONSTIF
#endif
namespace exlib {

	/*
		Represents the ring of integers mod mod
		Using Base as a base representation
	*/
	template<typename Base,unsigned long long mod>
	class mod_ring {
		Base _val;
	public:
		template<typename T>
		constexpr mod_ring(T&& val):_val(std::forward<T>(val))
		{
			_val%=mod;
		}
		constexpr mod_ring():_val()
		{}
	private:
		//used to construct when it is known that the input is already in modulo range
		struct no_mod_tag {};
		template<typename T>
		constexpr mod_ring(T&& val,no_mod_tag):_val(std::forward<T>(val))
		{}
	public:
#define comp(op) constexpr bool operator op (mod_ring const& o) const { return _val op o;}
		comp(<)
		comp(>)
		comp(<=)
		comp(>=)
		comp(==)
		comp(!=)
#undef comp
		constexpr operator Base const&() const
		{
			return _val;
		}
		constexpr mod_ring& operator*=(mod_ring const& o)
		{
			(_val*=o._val)%=mod;
			return *this;
		}
		constexpr mod_ring& operator+=(mod_ring const& o)
		{
			_val+=o._val;
			if(_val>=mod)
			{
				_val-=mod;
			}
			return *this;
		}
		constexpr mod_ring operator /=(mod_ring const& o) const
		{
			_val/=o._val;
			return *this;
		}
		constexpr mod_ring operator -=(mod_ring const& o) const
		{
			if(_val>=o._val)
			{
				_val-=o._val;
				return *this;
			}
			_val=mod-(o._val-_val);
			return *this;
		}
		constexpr mod_ring operator+(mod_ring const& o) const
		{
			Base t=_val+o._val;
			if(t>=mod) t-=mod;
			return mod_ring{std::move(t),no_mod_tag{}};
		}
		constexpr mod_ring operator*(mod_ring const& o) const
		{
			return mod_ring{o._val*_val};
		}
		constexpr mod_ring operator /(mod_ring const& o) const
		{
			return mod_ring{_val/o._val,no_mod_tag{}};
		}
		constexpr mod_ring operator-(mod_ring const& o) const
		{
			if(_val>=o._val)
			{
				return mod_ring{_val-o._val,no_mod_tag{}};
			}
			return mod_ring{mod-(o._val-_val),no_mod_tag{}};
		}
#define cassmd(op) static constexpr mod_ring operator op(mod_ring&& a,mod_ring const& o) { return std::move(a) op##= o;}
		cassmd(+)
		cassmd(-)
		cassmd(*)
		cassmd(/)
#undef cass
	};

	template<typename T>
	constexpr T additive_identity()
	{
		return T{0};
	}

	template<typename T>
	constexpr T multiplicative_identity()
	{
		return T{1};
	}

	//integral exponentiation a^n
	template<typename T>
	constexpr T ipow(T const& a,unsigned long long n)
	{
		if(n==0)
		{
			return multiplicative_identity<T>();
		}
		if(n==1)
		{
			return a;
		}
		auto x=a;
		auto y=multiplicative_identity<T>();
		do
		{
			if(n&1)
			{
				y*=x;
				x*=x;
				(n-	=1)/=2;
			}
			else
			{
				x*=x;
				n/=2;
			}
		} while(n>1);
		return x*y;
	}

	template<typename T>
	constexpr T abs(T const& a)
	{
		if(a<0)
		{
			return -a;
		}
		return a;
	}

	template<typename T>
	struct get_signed {
		typedef T type;
	};

#define _def_get_signed(ntype)\
	template<>\
	struct get_signed<unsigned ntype> {\
		typedef signed ntype type;\
	}
	_def_get_signed(char);
	_def_get_signed(short);
	_def_get_signed(int);
	_def_get_signed(long);
	_def_get_signed(long long);
#undef _def_get_signed
	namespace detail {
		template<typename R,typename T,typename U,typename V,typename W>
		struct min_modular_distance_type {
			typedef R type;
		};
		template<typename T,typename U,typename V,typename W>
		struct min_modular_distance_type<void,T,U,V,W> {
			typedef typename get_signed<typename std::common_type<T,U,V,W>::type>::type type;
		};
	}
	/*
		Finds the lowest magnitude c such that a+c=b in the modular group spanning [min,max),
		assuming a and b are in the modular group.
	*/
	template<typename R=void,typename T,typename U,typename V,typename W>
	constexpr auto min_modular_distance(T const& a,U const& b,V const& min,W const& max)
	{
		assert(min<=max);
		assert(a>=min&&a<=max);
		assert(b>=min&&b<=max);
		typedef typename detail::min_modular_distance_type<R,T,U,V,W>::type Ret;
		if(a==b)
		{
			return Ret{0};
		}
		if(a<b)
		{
			Ret forward_dist=b-a;
			Ret backward_dist=a-min+max-b;
			if(forward_dist<=backward_dist)
			{
				return forward_dist;
			}
			return backward_dist*=-1; //if they are allocating BigIntegers, prevents reallocation, hopefully
		}
		else
		{
			Ret forward_dist=max-a+b-min;
			Ret backward_dist=a-b;
			if(forward_dist<=backward_dist)
			{
				return forward_dist;
			}
			return backward_dist*=-1;
		}
	}

	/*
		Finds the lowest magnitude c such that a+c=b in the modular group spanning [0,max),
		assuming a and b are in the modular group.
	*/
	template<typename T,typename U,typename V>
	constexpr auto min_modular_distance(T const& a,U const& b,V const& max)
	{
		return min_modular_distance(a,b,V(0),max);
	}

	template<typename T,typename U>
	constexpr std::decay<U>::type clamp(T val,U&& min,U&& max)
	{
		assert(min<=max);
		if(val>max)
		{
			return std::forward<U>(max);
		}
		else if(val<min)
		{
			return std::forward<U>(min);
		}
		return std::move(val);
	}

	template<typename U,typename T>
	constexpr U clamp(T val)
	{
		if(val>std::numeric_limits<U>::max())
		{
			return std::numeric_limits<U>::max();
		}
		if(val<std::numeric_limits<U>::min())
		{
			return std::numeric_limits<U>::min();
		}
		return std::move(val);
	}

	template<typename T1,typename T2> constexpr auto abs_dif(T1 x,T2 y) ->
		decltype(x-y)
	{
		return (x>y?x-y:y-x);
	}

	//coerce forces VarType into FixedType by rounding
	template<typename FixedType,typename VarType>
	struct coerce_value {
		static constexpr FixedType coerce(VarType vt)
		{
			return FixedType(std::round(vt));
		}
	};

	//A value that has a fixed or variant value. If the value is not fixed (index!=fixed_index), its value is determined by multiplying
	//variant() by bases[index]
	//constexpr currently not allowed for unions so some of them do not work; just ignore for it as it doesn't cause a compiler error
	template<typename FixedType=int,typename VarType=float,typename IndexType=unsigned int,typename CoerceValue=coerce_value<FixedType,VarType>>
	struct maybe_fixed:protected CoerceValue {
	public:
		static constexpr IndexType const fixed_index=-1;
	private:
		struct ft_const {};
		struct vt_const {};
		IndexType _index;
		union {
			FixedType ft;
			VarType vt;
		};
	public:
		//fixed value at 0
		constexpr maybe_fixed():_index(fixed_index),ft(0)
		{}
		//creates a fixed value
		constexpr maybe_fixed(FixedType ft):_index(fixed_index),ft(ft)
		{}
		//creates a variable value with index
		constexpr maybe_fixed(VarType vt,IndexType index):_index(index),vt(vt)
		{}
		//whether the value is fixed
		constexpr bool fixed() const
		{
			return _index==fixed_index;
		}
		//the variable index
		constexpr IndexType index() const
		{
			return _index;
		}
		//sets the variable index
		constexpr void index(IndexType idx)
		{
			_index=idx;
		}
		//if the value is fixed, returns that fixed value. Otherwises returns bases[index()]*variant().
		//index value checked
		template<size_t N>
		constexpr FixedType value(std::array<FixedType,N> const& bases) const
		{
			if(_index==fixed_index)
			{
				return ft;
			}
			if(_index<N)
			{
				return CoerceValue::coerce(bases[_index]*vt);
			}
			throw std::out_of_range("Index too high");
		}
		//automatic conversion to array for convenience
		template<typename... Bases>
		constexpr FixedType value(Bases... bases) const
		{
			return value(std::array<FixedType,sizeof...(Bases)>{
				{
					bases...
				}});
		}

		//if the value is fixed, returns that fixed value. Otherwises returns bases[index()]*variant().
		//index value unchecked
		template<size_t N>
		constexpr FixedType operator()(std::array<FixedType,N> const& bases) const
		{
			if(_index==fixed_index)
			{
				return ft;
			}
			return CoerceValue::coerce(bases[_index]*vt);
		}
		//automatic conversion to array for convenience
		template<typename... Bases>
		constexpr FixedType operator()(Bases... bases) const
		{
			return operator()(std::array<FixedType,sizeof...(Bases)>{
				{
					bases...
				}});
		}

		//fixes the value to be ft
		constexpr void fix(FixedType ft)
		{
			_index=fixed_index;
			this->ft=ft;
		}

		//does nothing if value is already fixed, otherwises fixes the value to bases[index]*variant()
		template<size_t N>
		constexpr void fix_from(std::array<FixedType,N> const& bases)
		{
			if(_index==fixed_index) return;
			ft=operator()(bases);
			_index=fixed_index;
		}

		//automatic conversion to array for convenience
		template<typename... Bases>
		constexpr void fix_from(Bases... bases)
		{
			fix_from(std::array<FixedType,sizeof...(Bases)>{
				{
					bases...
				}});
		}

		//the variable amount, undefined if value is fixed
		constexpr VarType variant() const
		{
			return vt;
		}

		//sets the variable amount and basis to use
		constexpr void variant(VarType vt,IndexType index=1)
		{
			_index=index;
			this->vt=vt;
		}
	};
	enum class conv_error {
		none,out_of_range,invalid_characters
	};

	struct conv_res {
		conv_error ce;
		char const* last;
		operator bool() const
		{
			return ce!=conv_error::none;
		}
	};

	//the following parse algorithms take in null-terminated strings
	inline conv_res parse(char const* str,double& out)
	{
		int& err=errno;
		err=0;
		char* end;
		double res=std::strtod(str,&end);
		if(err==ERANGE)
		{
			return {conv_error::out_of_range,end};
		}
		if(end==str)
		{
			return {conv_error::invalid_characters,end};
		}
		out=res;
		return {conv_error::none,end};
	}

	inline conv_res parse(char const* str,float& out)
	{
		int& err=errno;
		err=0;
		char* end;
		float res=std::strtof(str,&end);
		if(err==ERANGE)
		{
			return {conv_error::out_of_range,end};
		}
		if(end==str)
		{
			return {conv_error::invalid_characters,end};
		}
		out=res;
		return {conv_error::none,end};
	}

	template<typename T>
	auto parse(char const* str,T& out,int base=10) -> typename std::enable_if<std::is_unsigned<T>::value,conv_res>::type
	{
		int& err=errno;
		err=0;
		char* end;
		unsigned long long res=std::strtoull(str,&end,base);
		if(err==ERANGE)
		{
			return {conv_error::out_of_range,end};
		}
		if(end==str)
		{
			return {conv_error::invalid_characters,end};
		}
		constexpr unsigned long long max=std::numeric_limits<T>::max();
		constexpr unsigned long long ullmax=std::numeric_limits<unsigned long long>::max();
		if EX_CONSTIF(max<ullmax)
		{
			if(res>max)
			{
				return {conv_error::out_of_range,end};
			}
		}
		out=res;
		return {conv_error::none,end};
	}

	template<typename T>
	auto parse(char const* str,T& out,int base=10) -> typename std::enable_if<std::is_signed<T>::value,conv_res>::type
	{
		int& err=errno;
		err=0;
		char* end;
		long long res=std::strtoll(str,&end,base);
		if(err==ERANGE)
		{
			return {conv_error::out_of_range,end};
		}
		if(end==str)
		{
			return {conv_error::invalid_characters,end};
		}

		constexpr long long max=std::numeric_limits<T>::max();
		constexpr long long llmax=std::numeric_limits<unsigned long long>::max();
		if EX_CONSTIF(max<llmax)
		{
			if(res>max)
			{
				return {conv_error::out_of_range,end};
			}
		}
		constexpr long long min=std::numeric_limits<T>::min();
		constexpr long long llmin=std::numeric_limits<unsigned long long>::min();
		if EX_CONSTIF(min>llmin)
		{
			if(res<min)
			{
				return {conv_error::out_of_range,end};
			}
		}
		out=res;
		return {conv_error::none,end};
	}

	template<typename T>
	EX_CONSTEXPR unsigned int num_digits(T num,unsigned int base=10)
	{
		assert(base>1);
		//static_assert(std::is_integral<T>::value,"Requires integral type");
		unsigned int num_digits=1;
		while((num/=base)!=0)
		{
			++num_digits;
		}
		return num_digits;
	}

#if __cplusplus>201700L

	namespace detail {
		template<typename CharType>
		constexpr auto make_digit_array()
		{
			std::array<CharType,'9'-'0'+1+'z'-'a'+1> arr{{}};
			size_t pos=0;
			for(CharType i='0';i<='9';++pos,++i)
			{
				arr[pos]=i;
			}
			for(CharType i='a';i<='z';++pos,++i)
			{
				arr[pos]=i;
			}
			return arr;
		}

		/*template<typename CharType>
		constexpr auto const& digit_array()
		{
			static constexpr auto arr=make_digit_array<CharType>();
			return arr;
		}*/

		template<typename CharType>
		struct digits_holder {
			static constexpr CharType digits[]=
			{'0','1','2','3','4','5','6','7','8','9',
			 'a','b','c','d','e','f','g','h','i','j',
			 'k','l','m','n','o','p','q','r','s','t',
			 'u','v','w','x','y','z'};
		};
	}

	template<auto val,int base,typename CharType=char>
	constexpr auto to_string()
	{
		using T=decltype(val);
		static_assert(std::is_integral_v<T>,"Integer type required");
		if constexpr(std::is_unsigned_v<T>||val>=0)
		{
			std::array<CharType,num_digits(val,base)+1> number{{}};
			auto v=val;
			number.back()='\0';
			auto it=number.end()-2;
			while(true)
			{
				*it=detail::digits_holder<CharType>::digits[v%base];
				v/=base;
				if(v==0)
				{
					break;
				}
				--it;
			};
			return number;
		}
		else
		{
			std::array<CharType,num_digits(val,base)+2> number{{}};
			auto v=val;
			number.back()='\0';
			number.front()='-';
			auto it=number.end()-2;
			while(true)
			{
				if constexpr(-1%base==-1)
				{
					*it=digits[(-(v%base))];
				}
				else
				{
					*it=digits[base-(v%base)];
				}
				v/=base;
				if(v==0)
				{
					break;
				}
				--it;
			}
			return number;
		}
	}

	template<auto val,typename CharType,int base=10>
	constexpr auto to_string()
	{
		return to_string<val,base,CharType>();
	}

	template<auto val>
	constexpr auto to_string()
	{
		return to_string<val,10,char>();
	}

	template<unsigned long long val,typename CharType=char>
	[[deprecated]] constexpr auto to_string_unsigned()
	{
		return to_string<val,CharType>();
	}


	namespace detail {
		/*
			Used because MSVC doesn't allow weak orderings in debug mode and get_fatten is more efficient
			if you choose the last least element.
		*/
		template<typename Iter,typename Comp>
		constexpr Iter min_element(Iter begin,Iter end,Comp c)
		{
#ifdef DEBUG
			if(begin==end)
			{
				return end;
			}
			auto smallest=begin;
			for(++begin!=end)
			{
				if(c(*begin,*smallest))
				{
					smallest=begin;
				}
			}
			return smallest;
#endif // DEBUG
			return std::min_element(begin,end,c);
		}
	}

#endif
	/*
		Makes a container in which each element becomes the minimum element within hp of its index
	*/
	template<typename RandomAccessContainer,typename Comp>
	RandomAccessContainer fattened_profile(RandomAccessContainer const& prof,size_t hp,Comp comp)
	{
		RandomAccessContainer res(prof.size());
		auto el=prof.begin()-1;
		auto rit=res.begin();
		for(auto it=prof.begin();it<prof.end();++it,++rit)
		{
			decltype(it) begin,end;
			if((it-prof.begin())<=hp)
			{
				begin=prof.begin();
			}
			else
			{
				begin=it-hp;
			}
			end=it+hp+1;
			if(end>prof.end())
			{
				end=prof.end();
			}
			if(el<begin)
			{
				el=std::min_element(begin,end,comp);
			}
			else
			{
				el=std::min(el,end-1,[=](auto const a,auto const b)
				{
					return comp(*a,*b);
				});
			}
			*rit=*el;
		}
		return res;
	}

	template<typename RandomAccessContainer>
	RandomAccessContainer fattened_profile(RandomAccessContainer const& prof,size_t hp)
	{
		typedef std::decay<decltype(*prof.begin())>::type T;
		if
#if __cplusplus > 201700L
			constexpr
#endif
			(std::is_trivially_copyable<T>::value&&sizeof(T)<=sizeof(size_t))
		{
			return fattened_profile(prof,hp,[](auto a,auto b)
			{
				return a<b;
			});
		}
		else
		{
			return fattened_profile(prof,hp,[](auto const& a,auto const& b)
			{
				return a<b;
			});
		}
	}

	template<typename T,typename Alloc=std::allocator<T>>
	class LimitedSet:std::vector<T,Alloc> {
	private:
		using Base=std::vector<T>;
	public:
		using Base::iterator;
		using Base::allocator_type;
		using Base::size_type;
		using Base::difference_type;
		using Base::const_reference;
		using Base::const_pointer;
		using Base::const_iterator;
		using Base::const_reverse_iterator;
	private:
		size_t _max_size;
	public:
		LimitedSet(size_t s):_max_size(s)
		{
			_data.reserve(s);
		}
		LimitedSet():LimitedSet(0)
		{}
		void max_size(size_t s)
		{
			_max_size=s;
			Base::reserve(s+1);
		}
		size_t max_size() const
		{
			return _max_size;
		}
		using Base::cbegin;
		using Base::cend;
		using Base::crbegin;
		using Base::crend;
		using Base::size;
		const_iterator begin() const
		{
			return Base::begin();
		}
		const_iterator end() const
		{
			return Base::begin();
		}
		const_iterator rbegin() const
		{
			return Base::rbegin();
		}
		const_iterator rend() const
		{
			return Base::rbegin();
		}
		using Base::empty;
		using Base::clear;
		using Base::pop_back;
		using Base::erase;
	private:
		template<typename U,typename Comp>
		void _insert(U&& in,Comp comp)
		{
			if(max_size)
			{
				auto const loc=std::lower_bound(Base::begin(),Base::end(),in,comp);
				if(size()>=max_size())
				{
					if(loc!=Base::end())
					{
						Base::erase(Base::end()-1);
						Base::insert(loc,std::move(in));
					}
				}
				else
				{
					Base::insert(loc,std::move(in));
				}
			}
		}
	public:
		template<typename Compare=std::less<T>>
		void insert(T&& in,Compare comp={})
		{
			_insert(std::move(in),comp);
		}
		template<typename Compare=std::less<T>>
		void insert(T const& in,Compare comp={})
		{
			_insert(in,comp);
		}
	};
	/*
		Little-endian arbitrarily sized 2's complement integer.
	*/
	class BigInteger {
		std::vector<int64_t> data;
	public:
		struct div_t;

		BigInteger(int64_t);

		template<typename T>
		friend void operator<<(std::basic_ostream<T>,BigInteger const&);

		size_t size() const;
		size_t capacity() const;
		void reserve(size_t);

		bool operator==(BigInteger const&) const;
		bool operator<(BigInteger const&) const;
		bool operator>(BigInteger const&) const;

		BigInteger operator+(BigInteger const&) const;
		BigInteger& operator+=(BigInteger const&);
		BigInteger operator-(BigInteger const&) const;
		BigInteger& operator-=(BigInteger const&);
		BigInteger operator-() const;
		BigInteger& negate();
		BigInteger operator*(BigInteger const&) const;
		BigInteger& operator*=(BigInteger const&);
		BigInteger operator/(BigInteger const&) const;
		BigInteger& operator/=(BigInteger const&);
		BigInteger operator%(BigInteger const&) const;
		BigInteger& operator%=(BigInteger const&);
		static div_t div(BigInteger const&,BigInteger const&);

		BigInteger operator>>(size_t) const;
		BigInteger& operator>>=(size_t);
		BigInteger operator<<(size_t) const;
		BigInteger& operator<<=(size_t);
		BigInteger operator&(BigInteger const&) const;
		BigInteger& operator&=(BigInteger const&);
		BigInteger operator|(BigInteger const&) const;
		BigInteger& operator|=(BigInteger const&);
		BigInteger operator^(BigInteger const&) const;
		BigInteger& operator^=(BigInteger const&);
		BigInteger operator~() const;
		BigInteger& bitwise_negate();

		int signum() const;

		operator int64_t() const;
		operator double() const;
	};

	struct BigInteger::div_t {
		BigInteger quot;
		BigInteger rem;
	};
}

namespace std {
	template<>
	exlib::BigInteger const& max<exlib::BigInteger>(exlib::BigInteger const&,exlib::BigInteger const&);

	template<>
	struct is_integral<exlib::BigInteger>:public std::true_type {};

	template<>
	struct is_signed<exlib::BigInteger>:public std::true_type {};
}

namespace exlib {
	inline BigInteger::BigInteger(int64_t val):data(5)
	{
		data.back()|=val;
		data.back()&=0x8000'0000'0000'0000;
		data.front()=val&0x7FFF'FFFF'FFFF'FFFF;
	}
	inline size_t BigInteger::size() const
	{
		return data.size();
	}
	inline size_t BigInteger::capacity() const
	{
		return data.capacity();
	}
	inline void BigInteger::reserve(size_t n)
	{
		auto old_back_val=data.back();
		auto& old_back=data.back();
		data.reserve(n);
		old_back&=0x7FFF'FFFF'FFFF'FFFF;
		data.back()|=old_back_val;
		data.back()&=0x8000'0000'0000'0000;
	}
	inline bool BigInteger::operator==(BigInteger const& other) const
	{
		if(signum()!=other.signum())
		{
			return false;
		}
		size_t limit=std::min(other.size(),size());
	}
}
#undef EX_CONSTIF
#undef EX_CONSTEXPR
#endif