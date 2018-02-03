#ifndef EXSTRING_H
#define EXSTRING_H
#ifndef CUSTSTRING_H
#define CUSTSTRING_H
#include <iostream>
#include <stdexcept>
#include <new>
#include <assert.h>
namespace exlib {
	template<typename T>
	size_t strlen(T const* p) {
		assert(p!=nullptr);
		size_t i=0;
		while(p[i]!=0)
		{
			++i;
		}
		return i;
	}
	template<typename T>
	class string_base {
	public:
		typedef T value_type;
		typedef T* pointer;
		typedef T const* const_pointer;
		typedef T& reference;
		typedef T const& const_reference;
		typedef T* iterator;
		typedef T const* const_iterator;
		typedef std::reverse_iterator<iterator> reverse_iterator;
		typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
		typedef size_t size_type;
		typedef ptrdiff_t difference_type;

		static size_type const npos=-1;
		static size_type const default_initial_capacity=8;
	private:
		pointer _data;
		size_type _size,_capacity;
		void move(string_base&&) noexcept;
	public:
		string_base();
		template<typename U>
		string_base(U const*);
		string_base(size_type size,T);
		template<typename U>
		string_base(U const*,size_type s);
		string_base(size_type capacity);
		string_base(string_base&&) noexcept;
		template<typename U>
		string_base(string_base<U> const&);

		template<typename U>
		string_base& operator=(string_base<U> const&);
		string_base& operator=(string_base&&) noexcept;

		bool operator==(string_base const&) const;
		~string_base();
		template<typename U>
		friend std::basic_ostream<U>& operator<<(std::basic_ostream<U>&,string_base<U> const&);
		iterator begin();
		const_iterator begin() const;
		iterator end();
		const_iterator cend() const;
		reverse_iterator rbegin();
		const_reverse_iterator crbegin() const;
		reverse_iterator rend();
		const_reverse_iterator crend() const;
		size_type size() const;
		size_type capacity() const;
		size_type max_size() const;
		void shrink_to_fit();
		bool empty() const;
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
		string_base& operator+=(string_base const&);
		string_base substring(size_type begin,size_type end) const;
		string_base substring(iterator begin,iterator end) const;

		size_type find(string_base const& target,size_type pos=0) const;
		size_type find(const_pointer target,size_type pos,size_type count) const;
		size_type find(const_pointer target,size_type pos=0) const;
		size_type find(T ch,size_type pos=0) const;

		reference operator[](size_type i) {
			return _data[i];
		}
		const_reference operator[](size_type i) const {
			return _data[i];
		}
		reference at(size_type i) {
			if(i>=size())
			{
				throw std::out_of_range();
			}
			return _data[i];
		}
		const_reference at(size_type i) const {
			return at(i);
		}
		reference front() {
			return _data[0];
		}
		const_reference front() const {
			return _data[0];
		}
		reference back() {
			return _data[size()-1];
		}
		const_reference back() const {
			return _data[size()-1];
		}
		pointer data() {
			return _data;
		}
		const_pointer data() const {
			return _data;
		}
		pointer c_str() {
			return _data;
		}
		const_pointer c_str() const {
			return _data;
		}

		void clear() {
			_size=0;
			_data[0]=0;
		}
	};
	typedef string_base<char> string;
	typedef string_base<wchar_t> wstring;

	template<typename T>
	template<typename U>
	string_base<T>::string_base(U const* cp): string_base(cp,cust::strlen(cp)) {}

	template<typename T>
	template<typename U>
	string_base<T>::string_base(
		U const* cp,
		typename string_base<T>::size_type s) {
		assert(cp!=nullptr);
		_size=s;
		_data=new T[s+1];
		for(_capacity=0;_capacity<_size;++_capacity)
		{
			_data[_capacity]=cp[_capacity];
		}
		_data[_capacity]=0;
	}
	template<typename T>
	string_base<T>::string_base(typename string_base<T>::size_type s,T c) {
		_size=s;
		_data=new typename string_base<T>::value_type[_size+1];
		for(_capacity=0;_capacity<_size;++_capacity)
		{
			_data[_capacity]=c;
		}
		_data[_capacity]=0;
	}
	template<typename T>
	string_base<T>::string_base(typename string_base<T>::size_type capacity):_capacity(capacity) {
		_data=new T[capacity+1];
		_data[0]=0;
		_size=0;
	}
	template<typename T>
	string_base<T>::string_base():_data(nullptr),_size(0),_capacity(0) {}
	template<typename T>
	template<typename U>
	string_base<T>::string_base(string_base<U> const& other):string_base(other.data()) {}

	template<typename T>
	void string_base<T>::move(string_base<T>&& other) noexcept {
		_data=other._data;
		_size=other._size;
		_capacity=other._capacity;
		other.release();
	}
	template<typename T>
	string_base<T>::string_base(string_base<T>&& other) noexcept {
		move(std::move(other));
	}
	template<typename T>
	string_base<T>::~string_base() {
		delete[] _data;
	}
	template<typename T>
	std::basic_ostream<T>& operator<<(std::basic_ostream<T>& os,string_base<T> const& str) {
		if(str._data==nullptr)
		{
			return os;
		}
		return os<<str._data;
	}
	template<typename T>
	typename string_base<T>::size_type string_base<T>::capacity() const {
		return _capacity;
	}
	template<typename T>
	typename string_base<T>::size_type string_base<T>::size() const {
		return _size;
	}

	template<typename T>
	template<typename U>
	string_base<T>& string_base<T>::operator=(string_base<U> const& other) {
		if(_capacity<other.size())
		{
			delete[] _data;
			_data=new T[other.size()+1];
			_capacity=other.size();
		}
		for(_size=0;_size<other.size();++_size)
		{
			_data[_size]=other[_size];
		}
		_data[_size]=0;
		return *this;
	}

	template<typename T>
	string_base<T>& string_base<T>::operator=(string_base<T>&& other) noexcept {
		assert(this!=&other);
		delete[] _data;
		move(std::move(other));
		return *this;
	}
	template<typename T>
	typename string_base<T>::iterator string_base<T>::begin() {
		return _data;
	}
	template<typename T>
	typename string_base<T>::const_iterator string_base<T>::begin() const {
		return begin();
	}
	template<typename T>
	typename string_base<T>::iterator string_base<T>::end() {
		return _data+_size;
	}
	template<typename T>
	typename string_base<T>::const_iterator string_base<T>::cend() const {
		return end();
	}
	template<typename T>
	typename string_base<T>::reverse_iterator string_base<T>::rbegin() {
		return string_base<T>::reverse_iterator(end());
	}
	template<typename T>
	typename string_base<T>::const_reverse_iterator string_base<T>::crbegin() const {
		return rbegin();
	}
	template<typename T>
	typename string_base<T>::reverse_iterator string_base<T>::rend() {
		return string_base<T>::reverse_iterator(begin());
	}
	template<typename T>
	typename string_base<T>::const_reverse_iterator string_base<T>::crend() const {
		return rend;
	}

	template<typename T>
	typename string_base<T>::size_type string_base<T>::max_size() const {
		return ~(0L);
	}
	template<typename T>
	void string_base<T>::shrink_to_fit() {
		if(_capacity==_size)
		{
			return;
		}
		T* np=new T[_size+1];
		for(_capacity=0;_capacity<_size;++_capacity)
		{
			np[_capacity]=_data[_capacity];
		}
		np[_capacity]=0;
		delete[] _data;
		_data=np;
	}
	template<typename T>
	bool string_base<T>::empty() const {
		return _size==0;
	}
	template<typename T>
	void string_base<T>::reserve(typename string_base<T>::size_type s) {
		if(_capacity>=s)
		{
			return;
		}
		T* np=new T[s+1];
		for(_capacity=0;_capacity<_size;++_capacity)
		{
			np[_capacity]=_data[_capacity];
		}
		np[_capacity]=0;
		_capacity=s;
		delete[] _data;
		_data=np;
	}

	template<typename T>
	void string_base<T>::push_back(T cr) {
		if(_size>=_capacity)
		{
			reserve(_capacity*2);
		}
		_data[_size]=cr;
		++_size;
	}
	template<typename T>
	void string_base<T>::pop_back() {
		_data[--_size]=0;
	}
	template<typename T>
	void string_base<T>::erase(typename string_base<T>::size_type pos) {
		--_size;
		while(pos<_size)
		{
			_data[pos]=_data[pos+1];
			++pos;
		}
		_data[pos]=0;
	}
	template<typename T>
	void string_base<T>::erase(typename string_base<T>::iterator pos) {
		erase(static_cast<typename string_base<T>::size_type>(pos-begin()));
	}
	template<typename T>
	void string_base<T>::erase(typename string_base<T>::iterator begin,typename string_base<T>::iterator end) {
		typedef typename string_base<T>::size_type st;
		erase(static_cast<st>(begin-_data),
			static_cast<st>(end-_data));
	}
	template<typename T>
	void string_base<T>::erase(typename string_base<T>::size_type begin,typename string_base<T>::size_type end) {
		size_t i,j;
		for(i=end,j=begin;i<_size;++i,++j)
		{
			_data[j]=_data[i];
		}
		_data[j]=0;
		_size=j;
	}
	template<typename T>
	string_base<T> string_base<T>::operator+(string_base<T> const& other) const {
		typedef typename string_base<T>::size_type st;
		string_base<T> ret;
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
	template<typename T>
	string_base<T>& string_base<T>::operator+=(string_base<T> const& other) {
		typedef typename string_base<T>::size_type st;
		reserve(size()+other.size());
		st limit=other.size();
		for(st i=0;i<limit;++i,++_size)
		{
			(*this)[_size]=other[i];
		}
		(*this)[_size]=0;
		return *this;
	}
	template<typename T>
	string_base<T> string_base<T>::substring(typename string_base<T>::size_type begin,typename string_base<T>::size_type end) const {
		string_base<T> ret;
		ret.reserve(end-begin);
		for(;begin<end;++begin,++ret._size)
		{
			ret[ret._size]=(*this)[begin];
		}
		ret[ret._size]=0;
		return ret;
	}
	template<typename T>
	string_base<T> string_base<T>::substring(typename string_base<T>::iterator begin,typename string_base<T>::iterator end) const {
		typedef typename string_base<T>::size_type st;
		return substring(st(begin-_data),st(end-_data));
	}

	template<typename T>
	bool string_base<T>::operator==(string_base<T> const& other) const {
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

	template<typename T>
	void string_base<T>::insert(
		typename string_base<T>::size_type pos,
		T c,
		typename string_base<T>::size_type count) {
		typedef typename string_base<T>::size_type st;
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

	template<typename T>
	typename string_base<T>::size_type string_base<T>::find(
		string_base<T> const& target,
		typename string_base<T>::size_type pos=0) const {
		return find(target.data(),pos,target.size());
	}

	template<typename T>
	typename string_base<T>::size_type string_base<T>::find(
		typename string_base<T>::const_pointer target,
		typename string_base<T>::size_type pos,
		typename string_base<T>::size_type count) const {
		typedef typename string_base<T>::size_type st;
		if(count==0)
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

	template<typename T>
	typename string_base<T>::size_type string_base<T>::find(
		typename string_base<T>::const_pointer target,
		typename string_base<T>::size_type pos=0) const {
		return find(target,pos,strlen(target));
	}

	template<typename T>
	typename string_base<T>::size_type string_base<T>::find(T ch,size_type pos=0) const {
		for(typename string_base<T>::size_type i=pos;i<size();++i)
		{
			if((*this)[i]==ch)
			{
				return i;
			}
		}
		return npos;
	}

	template<typename T>
	void string_base<T>::release() {
		_data=nullptr;
		_size=0;
		_capacity=0;
	}
}
#endif
#endif