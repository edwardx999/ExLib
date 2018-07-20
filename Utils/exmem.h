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
#ifndef EX_MEM_H
#define EX_MEM_H
#include <stdexcept>
#include <vector>
#include <type_traits>
#include <array>
#define FORCEINLINE __forceinline
namespace exlib {
	template<typename T>
	class array_view {
	public:
		using const_iterator=typename std::vector<typename T>::const_iterator;
		using const_reference=typename std::vector<typename T>::const_reference;
		using const_pointer=typename std::vector<typename T>::const_pointer;
		using value_type=T;
		using const_reverse_iterator=typename std::vector<typename T>::const_reverse_iterator;
		using size_type=typename std::vector<typename T>::size_type;
		using difference_type=typename std::vector<typename T>::difference_type;
	private:
		T* _data;
		size_t _size;
	public:
		constexpr array_view():_data(0),_size(0)
		{}
		array_view(std::vector<T> const& v):_data(v.data()),_size(v.size())
		{}
		constexpr array_view(T const* data,size_t size):_data(data),_size(size)
		{}
		template<size_t N>
		constexpr array_view(std::array<T,N> const& arr):_data(arr.data()),_size(arr.size())
		{}
		constexpr const_iterator begin() const noexcept
		{
			return const_iterator(_data);
		}
		constexpr const_iterator cbegin() const noexcept
		{
			return const_iterator(_data);
		}
		constexpr const_iterator end() const noexcept
		{
			return const_iterator(_data+size);
		}
		constexpr const_iterator cend() const noexcept
		{
			return const_iterator(_data+size);
		}
		constexpr const_reverse_iterator rbegin() const noexcept
		{
			return const_reverse_iterator(end());
		}
		constexpr const_reverse_iterator crbegin() const noexcept
		{
			return const_reverse_iterator(end());
		}
		constexpr const_reverse_iterator rend() const noexcept
		{
			return const_reverse_iterator(begin());
		}
		constexpr const_reverse_iterator crend() const noexcept
		{
			return const_reverse_iterator(begin());
		}
		constexpr const_reference operator[](size_t n) const
		{
			return _data[n];
		}
		constexpr const_reference at(size_t n) const
		{
			if(n>=_size)
			{
				throw std::out_of_range();
			}
			return _data[n];
		}
		constexpr const_reference front() const
		{
			return *_data;
		}
		constexpr const_reference back() const
		{
			return _data[_size-1];
		}
		constexpr const_pointer data() const noexcept
		{
			return _data;
		}
		constexpr size_t size() const noexcept
		{
			return _size;
		}
		constexpr size_t max_size() const noexcept
		{
			return std::numeric_limits<size_t>::max();
		}
		constexpr bool empty() const noexcept
		{
			return _size==0;
		}
		constexpr void data(T const* data) noexcept
		{
			_data=data;
		}
		constexpr void resize(size_t size) noexcept
		{
			_size=size;
		}
	};

	template<typename T,size_t size>
	class stack_buffer {
	private:
		size_t num;
		char data_buffer[size*sizeof(T)];
	public:
		typedef T& reference;
		typedef T const& const_reference;
		typedef typename std::vector<typename T>::iterator iterator;
		typedef typename std::vector<typename T>::const_iterator const_iterator;

		stack_buffer() noexcept:num(0)
		{}
		~stack_buffer()
		{
			if(!std::is_trivially_destructible<T>::value)
			{
				for(size_t i=0;i<num;++i)
				{
					reinterpret_cast<T*>(data_buffer)[i].~T();
				}
			}
		}
		reference front()
		{
			return *reinterpret_cast<T*>(data_buffer);
		}
		reference back()
		{
			return *(reinterpret_cast<T*>(data_buffer)+num-1);
		}
		reference operator[](size_t i)
		{
			return *(reinterpret_cast<T*>(data_buffer)+i);
		}
		reference at(size_t i)
		{
			if(i>=num)
			{
				throw std::out_of_range();
			}
			return (*this)[i];
		}
		iterator begin() noexcept
		{
			return iterator(reinterpret_cast<T*>(data_buffer));
		}
		iterator end() noexcept
		{
			return iterator(reinterpret_cast<T*>(data_buffer)+num);
		}
		void pop_back()
		{
			back().~T();
			--num;
		}
		void push_back_checked(T const& val)
		{
			if(num>=size)
			{
				throw std::out_of_range();
			}
			(reinterpret_cast<T*>(data_buffer))[num++]=val;
		}
		void push_back(T const& val) noexcept
		{
			(reinterpret_cast<T*>(data_buffer))[num++]=val;
		}
		size_t size() const noexcept
		{
			return num;
		}
		size_t max_size() const noexcept
		{
			return size;
		}
		template<typename... Args>
		void emplace_back(Args&&... args)
		{
			new (reinterpret_cast<T*>(data_buffer)+num++) T(std::forward<Args>(args)...);
		}
		template<typename... Args>
		void emplace_back_checked(Args&&... args)
		{
			if(num>=size)
			{
				throw std::out_of_range();
			}
			new (reinterpret_cast<T*>(data_buffer)+num++) T(std::forward<Args>(args)...);
		}
		T* data() noexcept
		{
			return reinterpret_cast<T*>(data_buffer);
		}
	};

	template<typename T>
	class stack_vector {
	private:
		char* _data;
		size_t _size;
		size_t _capacity;
	public:
		typedef T& reference;
		typedef T const& const_reference;
		typedef T* iterator;
		typedef T const* const_iterator;
		FORCEINLINE stack_vector(size_t s):_data(alloca(s)),_size(s),_capacity(s)
		{
			for(size_t i=0;i<_size;++i)
			{
				new (reinterpret_cast<T*>(data)+i) T();
			}
		}
		reference operator[](size_t s)
		{
			return (reinterpret_cast<T*>(data))[s];
		}
		reference at(size_t s)
		{
			if(s>=_size)
			{
				throw std::out_of_range();
			}
			return (*this)[s];
		}
		reference front()
		{
			return (*this)[0];
		}
		reference back()
		{
			return (*this)[_size-1];
		}
		T* data()
		{
			return _data;
		}
		iterator begin()
		{
			return (reinterpret_cast<T*>(data));
		}
		iterator end()
		{
			return (reinterpret_cast<T*>(data))+_size;
		}
		bool empty()
		{
			return _size==0;
		}
		size_t size() const
		{
			return _size;
		}
		size_t max_size() const
		{
			return ~0;
		}
		FORCEINLINE void reserve(size_t s)
		{
			if(s>_capacity)
			{
				alloca(s-_capacity);
			}
			_capacity=s;
		}
		size_t capacity() const
		{
			return _capacity;
		}
		void clear()
		{
			for(size_t i=0;i<_size;++i)
			{
				(reinterpret_cast<T*>(data))[i].~T();
			}
			_size=0;
		}
		~stack_vector()
		{
			clear();
		}
		template<typename... Args>
		FORCEINLINE void emplace(const_iterator pos,Args&&... args)
		{
			if(_size==_capacity)
			{
				reserve(2*_size);
			}
			++_size;
		}
		FORCEINLINE void insert(const_iterator pos,T const& val);

	};
}
#endif