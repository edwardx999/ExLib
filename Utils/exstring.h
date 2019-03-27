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
#include <iostream>
#include <stdexcept>
#include <new>
#include <assert.h>
#include <string>
#include <algorithm>
#include <limits>
#include <typeindex>
#include <utility>
//#include "exmeta.h"
namespace exlib {

	template<typename T,typename U>
	constexpr int strcmp(T const* a,U const* b)
	{
		for(size_t i=0;;++i)
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

	template<typename T>
	constexpr size_t strlen(T const* p)
	{
		assert(p!=nullptr);
		size_t i=0;
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
	String must have constructor that takes in (char* string,size_t count).
	Default buffer_size used to create the string is 15 (enough to fit 64 bit unsigned), increase if you need more.
	Giving negative values to this function will result in odd behavior.
	*/
	template<
		typename String=std::string,
		typename String::value_type alphabet_start='a',
		size_t alphabet_size='z'-alphabet_start+1,
		size_t buffer_size=15,
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
	String front_padded_string(String const& in,size_t numpadding,typename String::value_type padding)
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
	String back_padded_string(String const& in,size_t numpadding,typename String::value_type padding)
	{
		if(in.size()>=numpadding) return in;
		size_t padding_needed=numpadding-in.size();
		String out(in);
		out.insert(out.end(),padding_needed,padding);
		return out;
	}

	template<typename String>
	String& pad_back(String& in,size_t numpadding,typename String::value_type padding)
	{
		if(in.size()<numpadding)
		{
			size_t padding_needed=numpadding-in.size();
			in.insert(in.end(),padding_needed,padding);
		}
		return in;
	}

	constexpr inline char lowercase(char a)
	{
		if(a>='A'&&a<='Z')
		{
			return a+32;
		}
		return a;
	}

	template<typename T>
	constexpr int strncmp_nocase(T const* a,T const* b)
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
				if(int res=lowercase(*a)-lowercase(*b))
				{
					return res;
				}
			}
		}
		return 0;
	}

	template<typename T>
	constexpr bool is_digit(T ch)
	{
		return ch>='0'&&ch<='9';
	}

	template<typename T>
	constexpr int strncmp_num(T const* a_start,T const* a_end,T const* b_start,T const* b_end)
	{
		assert(a_start<=a_end);
		assert(b_start<=b_end);
		auto a_begin=a_start,b_begin=b_start;
		for(;a_begin!=a_end&&*a_begin=='0';++a_begin);//strip away leading zeros
		for(;b_begin!=b_end&&*b_begin=='0';++b_begin);
		if(int res=(a_end-a_begin)-(b_end-b_begin))
		{
			return res;
		}
		for(auto ab=a_begin,bb=b_begin;ab!=a_end;++ab,++bb)
		{
			if(int res=*ab-*bb)
			{
				return res;
			}
		}
		return int((b_begin-b_start)-(a_begin-a_start));
	}

	//comparison similar to windows sorting
	template<typename T>
	constexpr int strncmp_wind(T const* a,T const* b)
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

}
#endif