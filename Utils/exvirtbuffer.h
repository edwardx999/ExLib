#pragma once
#ifndef EXVIRTBUFFER_H
#define EXVIRTBUFFER_H
#include <cstddef>
#include <type_traits>
#include "extags.h"
#include <initializer_list>
#include <new>
#include <cstring>
namespace exlib {

	namespace virtbufdet {
		template<typename T>
		using remove_cvref_t=typename std::remove_cv<typename std::remove_reference<T>::type>::type;
	}

	/*
		Small object optimization for virtual types.
		Has similar interface to unique_ptr
	*/
	template<typename Base,std::size_t BufferSize=sizeof(Base),std::size_t Alignment=alignof(Base)>
	class virtual_buffer {
		static_assert(std::has_virtual_destructor<Base>::value,"Base class must have virtual constructor");
		Base* _data;
		typename std::aligned_storage<BufferSize,Alignment>::type _storage;
		template<typename B,std::size_t BS,std::size_t A>
		friend class virtual_buffer;
		template<typename T,typename... Args>
		Base* construct(std::integral_constant<bool,true>,Args&&... args) noexcept(std::is_nothrow_constructible<T,Args&&...>::value)
		{
			return new (&_storage) T(std::forward<Args>(args)...);
		}
		template<typename T,typename... Args>
		Base* construct(std::integral_constant<bool,false>,Args&&... args)
		{
			return new T(std::forward<Args>(args)...);
		}
		template<std::size_t s,std::size_t a>
		Base* move(virtual_buffer<Base,s,a>& other)
		{
			std::memcpy(&_storage,&other._storage,sizeof(other._storage));
			auto const odatap=reinterpret_cast<char*>(other._data);
			auto const ostors=reinterpret_cast<char*>(&other._storage);
			auto const ostore=ostors+sizeof(other._storage);
			auto const data=(odatap>=ostors&&odatap<ostore)? //in place
				reinterpret_cast<Base*>(reinterpret_cast<char*>(&_storage)+(odatap-ostors)):
				other._data;
			other._data=nullptr;
			return data;
		}

		template<typename T>
		using fits=std::integral_constant<bool,
			sizeof(T)<=BufferSize&&
			alignof(T)<=Alignment&&
			std::is_nothrow_move_constructible<T>::value>;

		void destruct()
		{
			if(_data)
			{
				if(reinterpret_cast<char*>(_data)>=reinterpret_cast<char*>(&_storage)&&
					reinterpret_cast<char*>(_data)<reinterpret_cast<char*>(&_storage)+sizeof(_storage))
				{
					_data->~Base();
				}
				else
				{
					delete _data;
				}
			}
		}
		template<typename T,typename... Args>
		void do_emplace(Args&&... args)
		{
			destruct();
#define _do_emplace_construct_call_ construct<T>(fits<T>{},std::forward<Args>(args)...)
			if
#ifdef __cpp_if_constexpr
				constexpr
#endif
				(noexcept(_do_emplace_construct_call_))
			{
				_data=_do_emplace_construct_call_;
			}
			else
			{
				try
				{
					_data=_do_emplace_construct_call_;
				}
				catch(...)
				{
					_data=nullptr;
					throw;
				}
			}
#undef _do_emplace_construct_call_
		}
	public:
		virtual_buffer() noexcept:_data{}
		{}

		virtual_buffer(virtual_buffer&& other) noexcept:_data{move(other)}
		{}

		virtual_buffer& operator=(virtual_buffer&& other) noexcept(std::is_nothrow_destructible<Base>::value)
		{
			destruct();
			_data=move(other);
			return *this;
		}

		template<std::size_t S,std::size_t A,typename=typename std::enable_if<S<=BufferSize&&A<=Alignment>::type>
		virtual_buffer(virtual_buffer<Base,S,A>&& other) noexcept:_data{move(other)}
		{}

		template<std::size_t S,std::size_t A>
		auto operator=(virtual_buffer<Base,S,A>&& other) noexcept(std::is_nothrow_destructible<Base>::value) -> typename std::enable_if<S<=BufferSize&&A<=Alignment,virtual_buffer&>::type
		{
			destruct();
			_data=move(other);
			return *this;
		}

		template<typename T,typename RCT=virtbufdet::remove_cvref_t<T>,typename=typename std::enable_if<std::is_convertible<RCT*,Base*>::value>::type>
		virtual_buffer(T&& derived_type):virtual_buffer(in_place_type_t<RCT>{},std::forward<T>(derived_type))
		{}

		template<typename T,typename=typename std::enable_if<std::is_convertible<T*,Base*>::value>::type>
		explicit virtual_buffer(T* own_me) noexcept:_data{own_me}
		{}

		template<typename T,typename... Args>
		virtual_buffer(in_place_type_t<T>,Args&&... args):_data{construct<T>(
				fits<T>{},
				std::forward<Args>(args)...)}
		{}

		template<typename T,typename U,typename... Args>
		virtual_buffer(in_place_type_t<T> t,std::initializer_list<U> list,Args&&... args):_data{construct<T>(
				fits<T>{},
				list,
				std::forward<Args>(args)...)}
		{}

		Base* operator->() noexcept
		{
			return _data;
		}
		Base& operator*() noexcept
		{
			return *_data;
		}
		Base* data() noexcept
		{
			return _data;
		}
		Base const* operator->() const noexcept
		{
			return _data;
		}
		Base const& operator*() const noexcept
		{
			return *_data;
		}
		Base const* data() const noexcept
		{
			return _data;
		}

		template<typename T,typename... Args>
		void emplace(Args&&... args)
		{
			do_emplace(std::forward<Args>(args)...);
		}

		template<typename T,typename U,typename... Args>
		void emplace(std::initializer_list<U> list,Args&&... args)
		{
			do_emplace<T>(list,std::forward<Args>(args)...);
		}

		template<typename T>
		auto reset(T* own_me) noexcept(std::is_nothrow_destructible<Base>::value) -> typename std::enable_if<std::is_convertible<T*,Base*>::value>::type
		{
			destruct();
			_data=own_me;
		}

		void reset() noexcept(std::is_nothrow_destructible<Base>::value)
		{
			reset(nullptr);
		}

		operator bool() const noexcept
		{
			return _data;
		}

		~virtual_buffer() noexcept(std::is_nothrow_destructible<Base>::value)
		{
			destruct();
		}
	};
}
#endif