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
#include <type_traits>
#define FORCEINLINE __forceinline
namespace exlib {
	template<typename T,size_t size>
	class stack_buffer {
	private:
		size_t num;
		char data_buffer[size*sizeof(T)];
	public:
		typedef T& reference;
		typedef T const& const_reference;
		typedef T* iterator;
		typedef T const* const_iterator;

		stack_buffer() noexcept:num(0) {}
		~stack_buffer() noexcept
		{
			if(!std::is_trivially_destructible<T>::value)
			{
				for(size_t i=0;i<num;++i)
				{
					reinterpret_cast<T*>(data_buffer)[i].~T();
				}
			}
		}
		reference front() noexcept
		{
			return *reinterpret_cast<T*>(data_buffer);
		}
		reference back() noexcept
		{
			return *(reinterpret_cast<T*>(data_buffer)+num-1);
		}
		reference operator[](size_t i) noexcept
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
			return reinterpret_cast<T*>(data_buffer);
		}
		iterator end() noexcept
		{
			return reinterpret_cast<T*>(data_buffer)+num;
		}
		void pop_back() noexcept
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
		void emplace_back(Args... args)
		{
			new (reinterpret_cast<T*>(data_buffer)+num++) T(args...);
		}
		template<typename... Args>
		void emplace_back_checked(Args... args)
		{
			if(num>=size)
			{
				throw std::out_of_range();
			}
			new (reinterpret_cast<T*>(data_buffer)+num++) T(args...);
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
		void emplace(const_iterator pos,Args... args)
		{
			if(_size==_capacity)
			{
				reserve(2*_size);
			}
			++_size;
		}
		void insert(const_iterator pos,T const& val);

	};
}
#endif