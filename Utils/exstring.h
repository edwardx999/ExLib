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
#ifndef EXSTRING_H
#define EXSTRING_H
#include <assert.h>
#include <cstddef>
#include <string>
#include <algorithm>
#include <limits>
#include <utility>
#include <iterator>
//#include "exmeta.h"
namespace exlib {

	template<typename T,typename U>
	constexpr int strcmp(T const* a,U const* b) noexcept
	{
		for(std::size_t i=0;;++i)
		{
			if(a[i]<b[i])
			{
				return -1;
			}
			if(a[i]>b[i])
			{
				return 1;
			}
			if(a[i]==0)
			{
				return 0;
			}
		}
	}

	template<typename T,typename U>
	constexpr bool strequal(T const* a,U const* b) noexcept
	{
		for(std::size_t i=0;;++i)
		{
			if(a[i]==0)
			{
				return b[i]==0;
			}
			if(a[i]!=b[i])
			{
				return false;
			}
		}
		return true;
	}

	template<typename T>
	constexpr std::size_t strlen(T const* p) noexcept
	{
		assert(p!=nullptr);
		std::size_t i=0;
		while(p[i]!=0)
		{
			++i;
		}
		return i;
	}

	/*
	Creates ordinal lettering such that:
	0	-> "a"
	1	-> "b"
	...
	25	-> "z"
	26	-> "aa"
	27	-> "ab"
	...
	51	-> "az"
	52	-> "ba"
	...
	701	-> "zz"
	702	-> "aaa"
	703	-> "aab"
	...
	String must have constructor that takes in (char* string,std::size_t count).
	Default buffer_size used to create the string is 15 (enough to fit 64 bit unsigned), increase if you need more.
	Giving negative values to this function will result in odd behavior.
	*/
	template<
		typename String=std::string,
		typename String::value_type alphabet_start='a',
		std::size_t alphabet_size='z'-alphabet_start+1,
		std::size_t buffer_size=15,
		typename N
	>
		String ordinal_lettering(N n)
	{
		static_assert(std::is_integral<N>::value,"Integral type required");
		char buffer[buffer_size];
		char* end=buffer+buffer_size;
		char* pos=end-1;
		while(true)
		{
			*pos=alphabet_start+n%alphabet_size;
			n/=alphabet_size;
			if(n<=0)
			{
				break;
			}
			n-=1;
			--pos;
		}
		return String(pos,end-pos);
	}

	template<typename String>
	String front_padded_string(String const& in,std::size_t numpadding,typename String::value_type padding)
	{
		if(in.size()>=numpadding) return in;
		size_t padding_needed=numpadding-in.size();
		String out(padding_needed,padding);
		out+=in;
		return out;
	}

	template<typename String>
	String& pad_front(String& in,size_t numpadding,typename String::value_type padding)
	{
		if(in.size()<numpadding)
		{
			size_t padding_needed=numpadding-in.size();
			in.insert(in.begin(),padding_needed,padding);
		}
		return in;
	}

	template<typename String>
	String back_padded_string(String const& in,std::size_t numpadding,typename String::value_type padding)
	{
		if(in.size()>=numpadding) return in;
		std::size_t padding_needed=numpadding-in.size();
		String out(in);
		out.insert(out.end(),padding_needed,padding);
		return out;
	}

	template<typename String>
	String& pad_back(String& in,std::size_t numpadding,typename String::value_type padding)
	{
		if(in.size()<numpadding)
		{
			std::size_t padding_needed=numpadding-in.size();
			in.insert(in.end(),padding_needed,padding);
		}
		return in;
	}

	constexpr inline char lowercase(char a) noexcept
	{
		if(a>='A'&&a<='Z')
		{
			return a+32;
		}
		return a;
	}

	template<typename T>
	constexpr int strncmp_nocase(T const* a,T const* b) noexcept
	{
		for(;;++a,++b)
		{
			if(*b==0)
			{
				return *a!=0?1:0;
			}
			else if(*a==0)
			{
				return -1;
			}
			else if(*b!=*a)
			{
				auto const la=lowercase(*a);
				auto const lb=lowercase(*b);
				if(la<lb) return -1;
				if(la>lb) return 1;
			}
		}
		return 0;
	}

	template<typename T>
	constexpr bool is_digit(T ch) noexcept
	{
		return ch>='0'&&ch<='9';
	}

	template<typename T>
	constexpr int strncmp_num(T const* a_start,T const* a_end,T const* b_start,T const* b_end) noexcept
	{
		assert(a_start<=a_end);
		assert(b_start<=b_end);
		auto a_begin=a_start,b_begin=b_start;
		for(;a_begin!=a_end&&*a_begin=='0';++a_begin);//strip away leading zeros
		for(;b_begin!=b_end&&*b_begin=='0';++b_begin);
		{
			std::size_t const anum_len=a_end-a_begin;
			std::size_t const bnum_len=b_end-b_begin;
			if(anum_len>bnum_len) return 1;
			if(anum_len<bnum_len) return -1;
			return 0;
		}
		for(auto ab=a_begin,bb=b_begin;ab!=a_end;++ab,++bb)
		{
			if(*ab>*bb) return 1;
			if(*ab<*bb) return -1;
		}
		std::size_t const blength=b_end-b_start;
		std::size_t const alength=a_end-a_start;
		if(alength>blength) return 1;
		if(alength<blength) return -1;
		return 0;
	}

	//comparison similar to windows sorting
	template<typename T>
	constexpr int strncmp_wind(T const* a,T const* b) noexcept
	{
		for(;;)
		{
			if(*b==0)
			{
				return *a==0?0:1;
			}
			if(*a==0)
			{
				return -1;
			}
			if(is_digit(*a)&&is_digit(*b))
			{
				auto asn=a;
				while(is_digit(*(++asn)));
				auto bsn=b;
				while(is_digit(*(++bsn)));
				if(int res=strncmp_num(a,asn,b,bsn))
				{
					return res;
				}
				a=asn;
				b=bsn;
				continue;
			}
			else if(*b!=*a)
			{
				if(int res=lowercase(*a)-lowercase(*b))
				{
					return res;
				}
			}
			++a,++b;
		}
	}

	template<typename Iter>
	struct unicode_conversion {
		std::uint32_t value;
		Iter next;
	};

	struct unicode_converter {
		template<typename Iter>
		auto operator()(Iter a) noexcept -> typename std::enable_if<std::is_same<typename std::decay<decltype(*a)>::type,char>::value,unicode_conversion<Iter>>::type
		{
			using Ret=unicode_conversion<Iter>;
			Ret ret;
			if(*a==0)
			{
				ret.next=a;
			}
			else if(*a<=0b01111111)
			{
				ret.value=*a;
				ret.next=++a;
			}
			else if(*a<=0b11011111) 
			{
				ret.value=(*a&0b00011111)<<6;
				++a;
				ret.value|=(*a&0b00111111);
				ret.next=++a;
			}
			else if(*a<=11101111)
			{
				ret.value=(*a&0b00001111)<<12;
				++a;
				ret.value|=(*a&0b00011111)<<6;
				++a;
				ret.value|=(*a&0b00111111);
				ret.next=++a;
			}
			else
			{
				ret.value=(*a&0b00000111)<<18;
				++a;
				ret.value|=(*a&0b00011111)<<12;
				++a;
				ret.value|=(*a&0b00011111)<<6;
				++a;
				ret.value|=(*a&0b00111111);
				ret.next=++a;
			}
			return ret;
		}
	};

	//unicode comparison, a converter is given its iter and return a struct containing next: the next valid iterator, and value,
	// the equivalent (utf32) value
	// if next==input, then the unicode sequence has ended
	template<typename CharIter1,typename CharIter2,typename Converter1=unicode_converter,typename Converter2=unicode_converter>
	int unicode_compare(CharIter1 a,CharIter2 b,Converter1 c1={},Converter2 c2={}) noexcept(noexcept(c1(a))&&noexcept(c2(b)))
	{
		while(true)
		{
			unicode_conversion a_step=c1(a);
			unicode_conversion b_step=c2(b);
			if(a_step.next==a)
			{
				if(b_step.next==b)
				{
					return 0;
				}
				return -1;
			}
			if(b_step.next==b)
			{
				return 1;
			}
			if(a_step.value<b_step.value)
			{
				return -1;
			}
			if(a_step.value>b_step.value)
			{
				return 1;
			}
			a=a_step.next;
			b=b_step.next;
		}
	}


	namespace string_buffer_detail {
		template<std::size_t N,typename Char>
		class string_buffer_base {
		protected:
			Char _data[N];
		public:
			using value_type=Char;
			using size_type=std::size_t;
			using difference_type=std::ptrdiff_t;
			using reference=Char&;
			using const_reference=Char const&;
			using pointer=Char*;
			using const_pointer=Char const*;
			using iterator=Char*;
			using const_iterator=Char const*;
			using reverse_iterator=std::reverse_iterator<Char*>;
			using const_reverse_iterator=std::reverse_iterator<Char const*>;
			
			constexpr pointer data() noexcept
			{
				return _data;
			}
			constexpr const_pointer data() const noexcept
			{
				return _data;
			}

			constexpr const_pointer c_str() const noexcept
			{
				return data();
			}

			constexpr iterator begin() noexcept
			{
				return _data;
			}
			constexpr const_iterator begin() const noexcept
			{
				return _data;
			}	
			constexpr const_iterator cbegin() const noexcept
			{
				return _data;
			}
			constexpr reverse_iterator rend() noexcept
			{
				return {begin()};
			}
			constexpr const_reverse_iterator rend() const noexcept
			{
				return {begin()};
			}
			constexpr const_reverse_iterator crend() const noexcept
			{
				return {begin()};
			}

			constexpr reference operator[](std::size_t s) noexcept
			{
				return _data[s];
			}
			constexpr value_type operator[](std::size_t s) const noexcept
			{
				return _data[s];
			}
			constexpr reference front() noexcept
			{
				return _data[0];
			}
			constexpr value_type front() const noexcept
			{
				return _data[0];
			}
			constexpr size_type max_size() const noexcept
			{
				return N-1;
			}
			constexpr size_type capacity() const noexcept
			{
				return max_size();
			}
			constexpr bool empty() const noexcept
			{
				return _data[0]==0;
			}

			constexpr void fill(Char value) noexcept
			{
				for(std::size_t=0;i<N-1;++i)
				{
					_data[i]=value;
				}
				_data[N-1]=0;
			}

			//assign value and return size of new string
			template<typename Iter>
			constexpr auto overwrite(Iter begin,Iter end) noexcept -> typename
				std::enable_if<
					std::is_convertible<
						typename std::iterator_traits<Iter>::iterator_category*,
						std::random_access_iterator_tag*
					>::value,
					size_type
				>::type
			{
				std::size_t dist=end-begin;
				if(dist>N-1)
				{
					end=begin+N-1;
					dist=N-1;
				}
				for(std::size_t i=0;i<dist;++i)
				{
					_data[i]=begin[i];
				}
				_data[dist]=0;
				return dist;
			}

			//assign value and return size of new string
			template<typename Iter>
			constexpr auto overwrite(Iter begin,Iter end) noexcept ->
				typename std::enable_if<
					!std::is_convertible<
						typename std::iterator_traits<Iter>::iterator_category*,
						std::random_access_iterator_tag*
					>::value,
					std::size_t
				>::type
			{
				std::size_t i=0;
				for(;i<N-1&&begin!=end;++begin,++i)
				{
					_data[i]=*begin;
				}
				_data[i]=0;
				return i;
			}

			//assign value and return size of new string
			template<typename OChar,std::size_t M>
			constexpr size_type overwrite(OChar(&buffer)[M]) noexcept
			{
				if(M>0)
				{
					auto const fixed_m=buffer[M-1]==0?M-1:M;
					auto const to_copy=fixed_m>N-1?N-1:fixed_m;
					for(std::size_t i=0;i<to_copy;++i)
					{
						_data[i]=buffer[i];
					}
					_data[to_copy]=0;
					return to_copy;
				}
				else
				{
					_data[0]=0;
					return 0;
				}
			}

			//assign value and return size of new string
			template<typename CharPointer>
			constexpr auto overwrite(CharPointer ptr) -> typename std::enable_if<std::is_pointer<CharPointer>::value,size_type>::type
			{
				std::size_t i=0;
				for(;i<N-1;++i)
				{
					if(ptr[i]==0)
					{
						break;
					}
					_data[i]=ptr[i];
				}
				_data[i]=0;
				return i;
			}
			
			constexpr size_type overwrite(std::initializer_list<Char> list) noexcept
			{
				auto const to_copy=list.size()<N?list.size():N-1;
				for(std::size_t i=0;i<to_copy;++i)
				{
					_data[i]=list.begin()[i];
				}
				return to_copy;
			}
		};

		template<std::size_t N,typename Char,typename Derived>
		class string_buffer_crtp_base:public string_buffer_base<N,Char> {
			constexpr std::size_t get_size() const noexcept
			{
				return static_cast<Derived const&>(*this).size();
			}
			constexpr void set_size(std::size_t i) noexcept
			{
				static_cast<Derived&>(*this).resize(i);
			}
			using Base=string_buffer_base<N,Char>;
		public:
			using typename Base::value_type;
			using typename Base::size_type;
			using typename Base::difference_type;
			using typename Base::reference;
			using typename Base::const_reference;
			using typename Base::pointer;
			using typename Base::const_pointer;
			using typename Base::iterator;
			using typename Base::const_iterator;
			using typename Base::reverse_iterator;
			using typename Base::const_reverse_iterator;

			template<typename Iter,typename=typename std::iterator_traits<Iter>::iterator_category>
			constexpr string_buffer_crtp_base(Iter begin,Iter end) noexcept
			{
				set_size(this->overwrite(begin,end));
			}

			template<typename CharPointer,typename=decltype(std::declval<Base&>().overwrite(std::declval<CharPointer const&>()))>
			constexpr string_buffer_crtp_base(CharPointer const& ptr) noexcept
			{
				set_size(this->overwrite(ptr));
			}

			constexpr string_buffer_crtp_base(std::initializer_list<Char> list) noexcept
			{
				set_size(this->overwrite(list));
			}

			constexpr reference at(std::size_t i)
			{
				if(i>=get_size()) throw std::out_of_range{};
				return (*this)[i];
			}
			constexpr value_type at(std::size_t i) const
			{
				if(i>=get_size()) throw std::out_of_range{};
				return (*this)[i];
			}

			constexpr reference back() noexcept
			{
				return (*this)[get_size()-1];
			}
			constexpr value_type back() const noexcept
			{
				return (*this)[get_size()-1];
			}

			constexpr iterator end() noexcept
			{
				return begin()+get_size();
			}
			constexpr const_iterator end() const noexcept
			{
				return begin()+get_size();
			}
			constexpr const_iterator cend() const noexcept
			{
				return end();
			}

			constexpr reverse_iterator rbegin() noexcept
			{
				return {end()};
			}
			constexpr const_reverse_iterator rbegin() const noexcept
			{
				return {end()};
			}
			constexpr const_reverse_iterator crbegin() const noexcept
			{
				return {end()};
			}

			template<typename CharPointer>
			constexpr auto operator=(CharPointer const& ptr) noexcept -> decltype(this->overwrite(ptr),std::declval<Derived&>())
			{
				set_size(this->overwrite(ptr));
				return static_cast<Derived&>(*this);
			}

			template<typename Iter>
			constexpr auto assign(Iter begin,Iter end) noexcept -> decltype(this->overwrite(begin,end),std::declval<Derived&>())
			{
				set_size(this->overwrite(begin,end));
				return static_cast<Derived&>(*this);
			}

			template<typename CharPointer>
			constexpr auto assign(CharPointer const& ptr) noexcept -> decltype(this->overwrite(ptr),std::declval<Derived&>())
			{
				set_size(this->overwrite(ptr));
				return static_cast<Derived&>(*this);
			}

			constexpr Derived& assign(std::initializer_list<Char> list) noexcept
			{
				set_size(this->overwrite(list));
				return static_cast<Derived&>(*this);
			}
		};

		template<std::size_t N>
		struct size_store {
		protected:
			using stored_size_type=typename exlib::smallest_representable_type<std::size_t,N>::type;
			stored_size_type _size{};
		};
	}

	// A fixed size buffer that can hold a string
	// An n-sized buffer will hold N-1 chars plus a null terminator
	template<std::size_t N,typename Char=char,bool store_size=true>
	class string_buffer:
		string_buffer_detail::size_store<N>,
		public string_buffer_detail::string_buffer_crtp_base<N,Char,string_buffer<N,Char,store_size>> {
		using Base=string_buffer_detail::string_buffer_crtp_base<N,Char,string_buffer<N,Char,store_size>>;
		using StoredSize=typename exlib::smallest_representable_type<std::size_t,N>::type;
	public:
		using constant_time_size=std::true_type;
		using Base::Base;
		using Base::operator=;
		constexpr string_buffer(string_buffer const& other) noexcept:Base(other.begin(),other.end()){}
		constexpr string_buffer& operator=(string_buffer const& other) noexcept
		{
			Base::assign(other.begin(),other.end());
			return *this;
		}
		constexpr void resize(typename Base::size_type new_size) noexcept
		{
			assert(new_size<N);
			this->_size=static_cast<StoredSize>(new_size);
		}
		constexpr typename Base::size_type size() const noexcept
		{
			return this->_size;
		}
	};

	template<std::size_t N,typename Char>
	class string_buffer<N,Char,false>:public string_buffer_detail::string_buffer_crtp_base<N,Char,string_buffer<N,Char,false>> {
		using Base=string_buffer_detail::string_buffer_crtp_base<N,Char,string_buffer<N,Char,false>>;
		friend Base;
		constexpr void resize(typename Base::size_type) noexcept
		{}
	public:
		constexpr string_buffer(string_buffer const& other) noexcept:Base(other.begin(),other.end())
		{}
		constexpr string_buffer& operator=(string_buffer const& other) noexcept
		{
			this->assign(other.begin(),other.end());
			return *this;
		}
		using constant_time_size=std::false_type;
		using Base::Base;
		constexpr typename Base::size_type size() const noexcept
		{
			return exlib::strlen(this->data());
		}
	};

	template<typename Char,bool ss>
	class string_buffer<0,Char,ss>:public string_buffer_detail::string_buffer_crtp_base<1,Char,string_buffer<0,Char,ss>> {
		using Base=string_buffer_detail::string_buffer_crtp_base<1,Char,string_buffer<0,Char,ss>>;
		friend Base;
		constexpr void resize(std::size_t) noexcept
		{}
	public:
		using constant_time_size=std::true_type;
		using Base::Base;
		constexpr typename Base::value_type size() const noexcept
		{
			return 0;
		}
	};

	template<typename OStream,std::size_t N,typename Char,bool store_size>
	auto operator<<(OStream& os,string_buffer<N,Char,store_size> const& buffer) -> decltype(os.write(buffer.data(),buffer.size()),std::declval<OStream&>())
	{
		os.write(buffer.data(),buffer.size());
		return os;
	}

	template<typename T>
	constexpr T* get_cstr(T* ptr) noexcept
	{
		return ptr;
	}

	template<typename CharPointer,typename T>
	constexpr auto get_cstr(T* ptr) noexcept -> typename std::enable_if<std::is_convertible<T*,CharPointer>::value,CharPointer>::value
	{
		return ptr;
	}

	template<typename T,typename... Extra>
	constexpr auto get_cstr(T const& container,Extra...) noexcept -> typename std::enable_if<sizeof...(Extra)==0,decltype(container.c_str())>::type
	{
		return container.c_str();
	}

	template<typename CharPointer,typename T,typename... Extra>
	constexpr auto get_cstr(T const& container,Extra...) noexcept -> typename std::enable_if<std::is_convertible<typename std::enable_if<sizeof...(Extra)==0,decltype(container.c_str())>::type,CharPointer>::value,CharPointer>::type
	{
		return container.c_str();
	}
	
#define exstring_string_buffer_comp(op)\
	template<typename String,std::size_t N,typename Char,bool store_size>\
	constexpr auto operator op(String const& str,string_buffer<N,Char,store_size> const& buf) noexcept -> decltype(get_cstr<Char const*>(str),true)\
	{\
		return exlib::strcmp(get_cstr<Char const*>(str),buf.data()) op 0;\
	}\
	template<std::size_t N,typename Char,bool store_size,typename String>\
	constexpr auto operator op(string_buffer<N,Char,store_size> const& buf,String const& str) noexcept -> decltype(get_cstr<Char const*>(str),true)\
	{\
		return exlib::strcmp(buf.data(),get_cstr<Char const*>(str)) op 0;\
	}\
	template<std::size_t N,typename Char,bool store_size,std::size_t M,bool store_size2>\
	constexpr bool operator op(string_buffer<N,Char,store_size> const& buf,string_buffer<M,Char,store_size2> const& str) noexcept\
	{\
		return exlib::strcmp(buf.data(),str.data()) op 0;\
	}
	exstring_string_buffer_comp(<)
	exstring_string_buffer_comp(>)
	exstring_string_buffer_comp(<=)
	exstring_string_buffer_comp(>=)
#undef exstring_string_buffer_comp
	
	template<std::size_t N,typename Char,bool store_size,typename String>
	constexpr auto operator==(string_buffer<N,Char,store_size> const& buf,String const& str) noexcept -> decltype(get_cstr<Char const*>(str),true)
	{
		constexpr auto constant_time_size=string_buffer<N,Char,store_size>::constant_time_size::value;
		auto const ptr=get_cstr<Char const*>(str);
		if(constant_time_size&&!std::is_pointer<String>::value)
		{
			return buf.size()==str.size()&&exlib::strequal(buf.c_str(),ptr);
		}
		else
		{
			return exlib::strequal(buf.c_str(),ptr);
		}
	}

	template<std::size_t N,typename Char,bool store_size,typename String>
	constexpr auto operator!=(string_buffer<N,Char,store_size> const& buf,String const& str) noexcept -> decltype(buf==str)
	{
		constexpr auto constant_time_size=string_buffer<N,Char,store_size>::constant_time_size::value;
		auto const ptr=get_cstr<Char const*>(str);
		if(constant_time_size&&!std::is_pointer<String>::value)
		{
			return buf.size()!=str.size()||!exlib::strequal(buf.c_str(),ptr);
		}
		else
		{
			return !exlib::strequal(buf.c_str(),ptr);
		}
	}

	template<typename String,std::size_t N,typename Char,bool store_size>
	constexpr auto operator==(String const& str,string_buffer<N,Char,store_size> const& buf) noexcept -> decltype(buf==str)
	{
		return buf==str;
	}

	template<typename String,std::size_t N,typename Char,bool store_size>
	constexpr auto operator!=(String const& str,string_buffer<N,Char,store_size> const& buf) noexcept -> decltype(buf!=str)
	{
		return buf!=str;
	}

	template<std::size_t N0,typename Char,bool SS0,std::size_t N1,bool SS1>
	constexpr bool operator==(string_buffer<N0,Char,SS0> const& buf1,string_buffer<N0,Char,SS1> const& buf2)
	{
		constexpr auto constant_time_size=
			string_buffer<N0,Char,SS0>::constant_time_size::value&&
			string_buffer<N1,Char,SS1>::constant_time_size::value;
		if(constant_time_size)
		{
			return buf1.size()==buf2.size()&&strequal(buf1.data(),buf2.data());
		}
		else
		{
			return strequal(buf1.data(),buf2.data());
		}
	}
	template<std::size_t N0,typename Char,bool SS0,std::size_t N1,bool SS1>
	constexpr bool operator!=(string_buffer<N0,Char,SS0> const& buf1,string_buffer<N0,Char,SS1> const& buf2)
	{
		constexpr auto constant_time_size=
			string_buffer<N0,Char,SS0>::constant_time_size::value&&
			string_buffer<N1,Char,SS1>::constant_time_size::value;
		if(constant_time_size)
		{
			return buf1.size()!=buf2.size()||!strequal(buf1.data(),buf2.data());
		}
		else
		{
			return !strequal(buf1.data(),buf2.data());
		}
	}
}
#endif