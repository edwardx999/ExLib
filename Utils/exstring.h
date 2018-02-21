/*
Copyright(C) 2017 Edward Xie

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <https://www.gnu.org/licenses/>.
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
#include "exmeta.h"
namespace exlib {
	template<typename T>
	size_t strlen(T const* p)
	{
		assert(p!=nullptr);
		size_t i=0;
		while(p[i]!=0)
		{
			++i;
		}
		return i;
	}

	template<typename T=char,typename CharT=std::char_traits<T>>
	class string_alg {
	public:
		typedef typename T value_type;
		typedef typename T* pointer;
		typedef typename T const* const_pointer;
		typedef typename T& reference;
		typedef typename T const& const_reference;
		typedef typename std::size_t size_type;
		typedef typename std::ptrdiff_t difference_type;

		typedef pointer iterator;
		typedef const_pointer const_iterator;
		typedef std::reverse_iterator<iterator> reverse_iterator;
		typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
	protected:
		pointer _data;
		size_type _size;
		string_alg(pointer data,size_type size):_data(data),_size(size) {}
		string_alg() {}
	public:
		static size_type const npos=-1;

		template<typename A,typename B>
		friend std::basic_ostream<A>& operator<<(std::basic_ostream<A>&,string_alg<A,B> const&);

		iterator begin() { return _data; }
		const_iterator begin() const { return _data; }
		iterator end() { return _data+_size; }
		const_iterator cend() const { return _data+_size; }
		reverse_iterator rbegin() { return reverse_iterator(end()); }
		const_reverse_iterator crbegin() const { return rbegin(); }
		reverse_iterator rend() { return reverse_iterator(begin()); }
		const_reverse_iterator crend() const { return rend(); }
		size_type size() const { return _size; };
		bool empty() const { return _size==0; }

		bool operator==(string_alg const&) const;
		bool operator!=(string_alg const&) const;
		bool operator>(string_alg const&) const;
		bool operator<(string_alg const&) const;
		bool operator>=(string_alg const&) const;
		bool operator<=(string_alg const&) const;

		size_type find(string_alg const& target,size_type pos=0) const;
		size_type find(const_pointer target,size_type pos,size_type count) const;
		size_type find(const_pointer target,size_type pos=0) const;
		size_type find(T ch,size_type pos=0) const;

		size_type rfind(T ch,size_type pos=npos) const;


		reference operator[](size_type i) { return _data[i]; }
		const_reference operator[](size_type i) const { return _data[i]; }
		reference at(size_type i)
		{
			if(i>=size()||i<0)
			{
				throw std::out_of_range();
			}
			return _data[i];
		}
		const_reference at(size_type i) const
		{
			if(i>=size()||i<0)
			{
				throw std::out_of_range();
			}
			return _data[i];
		}
		reference front() { return _data[0]; }
		const_reference front() const { return _data[0]; }
		reference back()
		{
			return _data[size()-1];
		}
		const_reference back() const
		{
			return _data[size()-1];
		}
		pointer data()
		{
			return _data;
		}
		const_pointer data() const
		{
			return _data;
		}
		pointer c_str()
		{
			return _data;
		}
		const_pointer c_str() const
		{
			return _data;
		}
	};

	template<typename T,typename CharT=std::char_traits<T>,typename Alloc=std::allocator<T>>
	class string_base:public string_alg<T,CharT>,protected Alloc {
	public:
		typedef typename string_alg<T,CharT>::value_type value_type;
		typedef typename string_alg<T,CharT>::pointer pointer;
		typedef typename string_alg<T,CharT>::const_pointer const_pointer;
		typedef typename string_alg<T,CharT>::reference reference;
		typedef typename string_alg<T,CharT>::const_reference const_reference;
		typedef typename string_alg<T,CharT>::size_type size_type;
		typedef typename string_alg<T,CharT>::difference_type difference_type;

		typedef typename string_alg<T,CharT>::iterator iterator;
		typedef typename string_alg<T,CharT>::const_iterator const_iterator;
		typedef typename string_alg<T,CharT>::reverse_iterator reverse_iterator;
		typedef typename string_alg<T,CharT>::const_reverse_iterator const_reverse_iterator;
	private:
		size_type _capacity;
		void move(string_base&&) noexcept;
		void reallocate(size_type);
	public:
		string_base();
		template<typename U>
		string_base(U const*);
		string_base(size_type size,T);
		template<typename U>
		string_base(U const*,size_type s);
		string_base(size_type capacity);
		string_base(string_base&&) noexcept;

		template<typename String>
		string_base(String const&);
		string_base(string_base const& other);

		template<typename String>
		string_base& operator=(String const&);
		string_base& operator=(string_base const& other);
		template<typename U>
		string_base& operator=(U const*);
		string_base& operator=(string_base&&) noexcept;

		~string_base();
		size_type capacity() const { return _capacity; }
		void shrink_to_fit();
		void resize(size_type);
		void reserve(size_type);
		void release();

		void push_back(T);
		void pop_back();

		void erase(iterator pos);
		void erase(size_type pos);
		void erase(iterator begin,iterator end);
		void erase(size_type begin,size_type end);

		void insert(size_type pos,T c,size_type count=1);

		string_base operator+(string_base const&) const;
		template<typename U,typename CharU>
		string_base& operator+=(string_alg<U,CharU> const&);
		template<typename U,typename CharU,typename AllocU>
		string_base& operator+=(string_base<U,CharU,AllocU> const&);
		string_base substr(size_type begin,size_type end) const;
		string_base substr(iterator begin,iterator end) const;

		void clear()
		{
			_size=0;
			_data[0]=0;
		}
	};
	typedef string_base<char> string;
	typedef string_base<wchar_t> wstring;

	/*
		Holds a ptr to a c string with its size.
		Has no control of the c string.
		Unsafe if underlying string is moved.
	*/
	template<typename T,typename CharT=std::char_traits<T>>
	class weak_string_base:public string_alg<T,CharT> {
	public:
		typedef typename string_alg<T,CharT>::value_type value_type;
		typedef typename string_alg<T,CharT>::pointer pointer;
		typedef typename string_alg<T,CharT>::const_pointer const_pointer;
		typedef typename string_alg<T,CharT>::reference reference;
		typedef typename string_alg<T,CharT>::const_reference const_reference;
		typedef typename string_alg<T,CharT>::size_type size_type;
		typedef typename string_alg<T,CharT>::difference_type difference_type;

		typedef typename string_alg<T,CharT>::iterator iterator;
		typedef typename string_alg<T,CharT>::const_iterator const_iterator;
		typedef typename string_alg<T,CharT>::reverse_iterator reverse_iterator;
		typedef typename string_alg<T,CharT>::const_reverse_iterator const_reverse_iterator;
		weak_string_base(T* data):string_alg(data,exlib::strlen(data)) {}
	};
	typedef weak_string_base<char> weak_string;
	typedef weak_string_base<wchar_t> weak_wstring;

	template<typename T,typename CharT,typename Alloc>
	template<typename U>
	string_base<T,CharT,Alloc>::string_base(U const* cp): string_base(cp,exlib::strlen(cp)) {}

	template<typename T,typename CharT,typename Alloc>
	template<typename U>
	string_base<T,CharT,Alloc>::string_base(
		U const* cp,
		typename string_base<T,CharT,Alloc>::size_type s)
	{
		assert(cp!=nullptr);
		_size=s;
		_data=allocate(s+1);
		for(_capacity=0;_capacity<_size;++_capacity)
		{
			_data[_capacity]=cp[_capacity];
		}
		_data[_capacity]=0;
	}
	template<typename T,typename CharT,typename Alloc>
	string_base<T,CharT,Alloc>::string_base(typename string_base<T,CharT,Alloc>::size_type s,T c)
	{
		_size=s;
		_data=allocate(s+1);
		for(_capacity=0;_capacity<_size;++_capacity)
		{
			_data[_capacity]=c;
		}
		_data[_capacity]=0;
	}
	template<typename T,typename CharT,typename Alloc>
	string_base<T,CharT,Alloc>::string_base(typename string_base<T,CharT,Alloc>::size_type capacity):_capacity(capacity)
	{
		_data=allocate(_capacity+1);
		_data[0]=0;
		_size=0;
	}
	template<typename T,typename CharT,typename Alloc>
	string_base<T,CharT,Alloc>::string_base():string_alg<T,CharT>(nullptr,0),_capacity(0) {}

	template<typename T,typename CharT,typename Alloc>
	template<typename String>
	string_base<T,CharT,Alloc>::string_base(String const& other):string_base(other.data(),other.size()) {}

	template<typename T,typename CharT,typename Alloc>
	string_base<T,CharT,Alloc>::string_base(string_base<T,CharT,Alloc> const& other):string_base(other.data(),other.size()) {}

	template<typename T,typename CharT,typename Alloc>
	void string_base<T,CharT,Alloc>::reallocate(typename string_base<T,CharT,Alloc>::size_type s)
	{
		typedef typename string_base<T,CharT,Alloc>::size_type st;
		typedef typename string_base<T,CharT,Alloc>::pointer pointer;
		pointer np=allocate(s+1);
		st i;
		for(i=0;i<_size;++i)
		{
			np[i]=_data[i];
		}
		np[i]=0;
		deallocate(_data,_capacity+1);
		_data=np;
		_capacity=s;
	}

	template<typename T,typename CharT,typename Alloc>
	void string_base<T,CharT,Alloc>::move(string_base<T,CharT,Alloc>&& other) noexcept
	{
		_data=other.data();
		_size=other.size();
		_capacity=other.capacity();
		other.release();
	}
	template<typename T,typename CharT,typename Alloc>
	string_base<T,CharT,Alloc>::string_base(string_base<T,CharT,Alloc>&& other) noexcept
	{
		move(std::move(other));
	}
	template<typename T,typename CharT,typename Alloc>
	string_base<T,CharT,Alloc>::~string_base()
	{
		deallocate(_data,_capacity+1);
	}

	template<typename U,typename CharU>
	std::basic_ostream<U>& operator<<(std::basic_ostream<U>& os,string_alg<U,CharU> const& str)
	{
		if(str._data==nullptr)
		{
			return os;
		}
		return os<<str._data;
	}

	template<typename T,typename CharT,typename Alloc>
	template<typename String>
	string_base<T,CharT,Alloc>& string_base<T,CharT,Alloc>::operator=(String const& other)
	{
		reserve(other.size());
		typename string_base<T,CharT,Alloc>::size_type s;
		for(s=0;s<other.size();++s)
		{
			_data[s]=other[s];
		}
		_data[s]=0;
		_size=other.size();
		return *this;
	}

	template<typename T,typename CharT,typename Alloc>
	string_base<T,CharT,Alloc>& string_base<T,CharT,Alloc>::operator=(string_base<T,CharT,Alloc> const& other)
	{
		reserve(other.size());
		typename string_base<T,CharT,Alloc>::size_type s;
		for(s=0;s<other.size();++s)
		{
			_data[s]=other[s];
		}
		_data[s]=0;
		_size=other.size();
		return *this;
	}

	template<typename T,typename CharT,typename Alloc>
	template<typename U>
	string_base<T,CharT,Alloc>& string_base<T,CharT,Alloc>::operator=(U const* cp)
	{
		_size=exlib::strlen(cp);
		reserve(_size);
		typename string_base<T,CharT,Alloc>::size_type s;
		for(s=0;s<_size;++s)
		{
			_data[s]=cp[s];
		}
		_data[s]=0;
		return *this;
	}

	template<typename T,typename CharT,typename Alloc>
	string_base<T,CharT,Alloc>& string_base<T,CharT,Alloc>::operator=(string_base<T,CharT,Alloc>&& other) noexcept
	{
		assert(this!=&other);
		deallocate(_data,_capacity+1);
		move(std::move(other));
		return *this;
	}

	template<typename T,typename CharT,typename Alloc>
	void string_base<T,CharT,Alloc>::shrink_to_fit()
	{
		typedef typename string_base<T,CharT,Alloc>::size_type st;
		if(_capacity==_size)
		{
			return;
		}
		T* np=allocate(_size+1);
		st i;
		for(i=0;i<_size;++i)
		{
			np[i]=_data[i];
		}
		np[i]=0;
		deallocte(_data,_capacity+1);
		_capacity=_size;
		_data=np;
	}

	template<typename T,typename CharT,typename Alloc>
	void string_base<T,CharT,Alloc>::resize(typename string_base<T,CharT,Alloc>::size_type s)
	{
		reserve(s);
		_size=s;
		_data[s]=0;
	}

	template<typename T,typename CharT,typename Alloc>
	void string_base<T,CharT,Alloc>::reserve(typename string_base<T,CharT,Alloc>::size_type s)
	{
		if(_capacity>s&&_data!=nullptr)
		{
			return;
		}
		reallocate(s);
	}

	template<typename T,typename CharT,typename Alloc>
	void string_base<T,CharT,Alloc>::push_back(T cr)
	{
		if(_size>=_capacity)
		{
			reallocate(2*_size+1);
		}
		_data[_size]=cr;
		++_size;
	}

	template<typename T,typename CharT,typename Alloc>
	void string_base<T,CharT,Alloc>::pop_back()
	{
		_data[--_size]=0;
	}

	template<typename T,typename CharT,typename Alloc>
	void string_base<T,CharT,Alloc>::erase(typename string_base<T,CharT,Alloc>::size_type pos)
	{
		--_size;
		while(pos<_size)
		{
			_data[pos]=_data[pos+1];
			++pos;
		}
		_data[pos]=0;
	}

	template<typename T,typename CharT,typename Alloc>
	void string_base<T,CharT,Alloc>::erase(typename string_base<T,CharT,Alloc>::iterator pos)
	{
		erase(static_cast<typename string_base<T,CharT,Alloc>::size_type>(pos-begin()));
	}

	template<typename T,typename CharT,typename Alloc>
	void string_base<T,CharT,Alloc>::erase(
		typename string_base<T,CharT,Alloc>::iterator begin,
		typename string_base<T,CharT,Alloc>::iterator end)
	{
		typedef typename string_base<T,CharT,Alloc>::size_type st;
		erase(static_cast<st>(begin-_data),
			static_cast<st>(end-_data));
	}

	template<typename T,typename CharT,typename Alloc>
	void string_base<T,CharT,Alloc>::erase(
		typename string_base<T,CharT,Alloc>::size_type begin,
		typename string_base<T,CharT,Alloc>::size_type end)
	{
		typename string_base<T,CharT,Alloc>::size_type i,j;
		for(i=end,j=begin;i<_size;++i,++j)
		{
			_data[j]=_data[i];
		}
		_data[j]=0;
		_size=j;
	}

	template<typename T,typename CharT,typename Alloc>
	string_base<T,CharT,Alloc> string_base<T,CharT,Alloc>::operator+(string_base<T,CharT,Alloc> const& other) const
	{
		typedef typename string_base<T,CharT,Alloc>::size_type st;
		string_base<T,CharT,Alloc> ret;
		ret.reserve(size()+other.size());
		for(;ret._size<size();++ret._size)
		{
			ret[ret._size]=(*this)[ret._size];
		}
		st j=0;
		for(;j<other.size();++j,++ret._size)
		{
			ret[ret._size]=other[j];
		}
		ret[ret._size]=0;
		return ret;
	}

	template<typename T,typename CharT,typename Alloc>
	template<typename U,typename CharU>
	string_base<T,CharT,Alloc>& string_base<T,CharT,Alloc>::operator+=(string_alg<U,CharU> const& other)
	{
		typedef typename string_base<T,CharT,Alloc>::size_type st;
		reserve(size()+other.size());
		st limit=other.size();
		for(st i=0;i<limit;++i,++_size)
		{
			(*this)[_size]=other[i];
		}
		(*this)[_size]=0;
		return *this;
	}

	template<typename T,typename CharT,typename Alloc>
	template<typename U,typename CharU,typename AllocU>
	string_base<T,CharT,Alloc>& string_base<T,CharT,Alloc>::operator+=(string_base<U,CharU,AllocU> const& other)
	{
		return ((*this)+=static_cast<string_alg<U,CharU> const&>(other));
	}

	template<typename T,typename CharT,typename Alloc>
	string_base<T,CharT,Alloc> string_base<T,CharT,Alloc>::substr(
		typename string_base<T,CharT,Alloc>::size_type begin,
		typename string_base<T,CharT,Alloc>::size_type end) const
	{
		string_base<T,CharT,Alloc> ret;
		ret.reserve(end-begin);
		for(;begin<end;++begin,++ret._size)
		{
			ret[ret._size]=(*this)[begin];
		}
		ret[ret._size]=0;
		return ret;
	}

	template<typename T,typename CharT,typename Alloc>
	string_base<T,CharT,Alloc> string_base<T,CharT,Alloc>::substr(
		typename string_base<T,CharT,Alloc>::iterator begin,
		typename string_base<T,CharT,Alloc>::iterator end) const
	{
		typedef typename string_base<T,CharT,Alloc>::size_type st;
		return substr(static_cast<st>(begin-_data),static_cast<st>(end-_data));
	}

	template<typename T,typename CharT>
	bool string_alg<T,CharT>::operator==(string_alg<T,CharT> const& other) const
	{
		if(other.size()==size())
		{
			for(size_t i=0;i<size();++i)
			{
				if((*this)[i]!=other[i])
				{
					return false;
				}
			}
			return true;
		}
		return false;
	}

	template<typename T,typename CharT,typename Alloc>
	void string_base<T,CharT,Alloc>::insert(
		typename string_base<T,CharT,Alloc>::size_type pos,
		T c,
		typename string_base<T,CharT,Alloc>::size_type count)
	{
		typedef typename string_base<T,CharT,Alloc>::size_type st;
		st new_size=count+_size;
		reserve(new_size);
		_data[new_size]=0;
		st i,j;
		for(i=new_size-1,j=_size-1;;--i,--j)
		{
			_data[i]=_data[j];
			if(j==pos)
			{
				break;
			}
		}
		i=pos+count;
		for(;j<i;++j)
		{
			_data[j]=c;
		}
	}

	template<typename T,typename CharT>
	typename string_alg<T,CharT>::size_type string_alg<T,CharT>::find(
		string_alg<T,CharT> const& target,
		typename string_alg<T,CharT>::size_type pos) const
	{
		return find(target.data(),pos,target.size());
	}

	template<typename T,typename CharT>
	typename string_alg<T,CharT>::size_type string_alg<T,CharT>::find(
		typename string_alg<T,CharT>::const_pointer target,
		typename string_alg<T,CharT>::size_type pos,
		typename string_alg<T,CharT>::size_type count) const
	{
		typedef typename string_alg<T,CharT>::size_type st;
		if(count==0||pos>size())
		{
			return npos;
		}
		st space_size=size()-pos;
		if(count>space_size)
		{
			return npos;
		}
		st limit=size()-count;
		for(st i=pos;i<=limit;++i)
		{
			if((*this)[i]==target[0])
			{
				for(st tpos=1;tpos<count;++tpos)
				{
					if((*this)[i+tpos]!=target[tpos])
					{
						i=i+tpos+1;
						break;
					}
				}
				return i;
			}
		}
		return npos;
	}

	template<typename T,typename CharT>
	typename string_alg<T,CharT>::size_type string_alg<T,CharT>::find(
		typename string_alg<T,CharT>::const_pointer target,
		typename string_alg<T,CharT>::size_type pos) const
	{
		return find(target,pos,exlib::strlen(target));
	}

	template<typename T,typename CharT>
	typename string_alg<T,CharT>::size_type string_alg<T,CharT>::find(
		T ch,
		typename string_alg<T,CharT>::size_type pos) const
	{
		for(typename string_base<T>::size_type i=pos;i<size();++i)
		{
			if((*this)[i]==ch)
			{
				return i;
			}
		}
		return npos;
	}

	template<typename T,typename CharT,typename Alloc>
	void string_base<T,CharT,Alloc>::release()
	{
		_data=nullptr;
		_size=0;
		_capacity=0;
	}

	template<typename T,typename CharT>
	typename string_alg<T,CharT>::size_type string_alg<T,CharT>::rfind(
		T ch,
		typename string_alg<T,CharT>::size_type pos) const
	{
		if(pos>_size)
		{
			pos=_size;
		}
		pos=pos-1;
		do
		{
			if(_data[pos]==ch)
			{
				return pos;
			}
			--pos;
		} while(pos>0);
		return npos;
	}
}
#endif