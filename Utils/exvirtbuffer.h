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
	namespace virtbuffdet {
		class seppekuer {
			void* _loc;
		public:
			~seppekuer() noexcept
			{
				operator delete(_loc);
			}
			void reset(void* loc=nullptr) noexcept
			{
				_loc=loc;
			}
		};
		//A derived class that deletes itself
		template<typename Derived>
		class self_deleting_derived:public seppekuer,public Derived {
		public:
			template<typename... Args>
			self_deleting_derived(Args&&... args)try: Derived(std::forward<Args>(args)...)
			{
			}
			catch(...)
			{
				seppekuer::reset();
				throw;
			}
		};
		template<typename T>
		using remove_cvref_t=typename std::remove_cv<typename std::remove_reference<T>::type>::type;
	}
	template<typename Base,std::size_t BufferSize=sizeof(Base),std::size_t Alignment=alignof(Base)>
	class virtual_buffer {
		static_assert(std::has_virtual_destructor<Base>::value,"Base class must have virtual constructor");
		Base* _data;
		typename std::aligned_storage<BufferSize,Alignment>::type _storage;
		template<typename B,std::size_t BS,std::size_t A>
		friend class virtual_buffer;
		template<typename T,typename... Args>
		Base* construct(std::integral_constant<bool,true>,Args&&... args)
		{
			return new (&_storage) T(std::forward<Args>(args)...);
		}
		template<typename T,typename... Args>
		Base* construct(std::integral_constant<bool,false>,Args&&... args)
		{
			using D=virtbuffdet::self_deleting_derived<T>;
			using Buff=typename std::aligned_storage<sizeof(D),alignof(D)>::type;
			constexpr std::size_t space=sizeof(D);
#ifndef __cpp_aligned_new
			static_assert(alignof(T)<=sizeof(std::max_align_t),"Over-aligned allocation not supported");
#endif
#if defined(_WIN32) && !defined(_EXVIRTBUFFER_ENABLE_OVERALIGNMENT)
			static_assert(alignof(T)<=16,"Because of how Windows memory allocation works, alignment cannot be over 16. If this is fixed in the future, define _EXVIRTBUFFER_ENABLE_OVERALIGNMENT");
#endif
			auto bptr=new Buff;
			auto ptr=reinterpret_cast<D*>(bptr);
			if(std::is_nothrow_constructible<T,Args&&...>::value)
			{
				new (bptr) virtbuffdet::self_deleting_derived<T>(std::forward<Args>(args)...);
			}
			else
			{
				try
				{
					new (bptr) virtbuffdet::self_deleting_derived<T>(std::forward<Args>(args)...);
				}
				catch(...)
				{
					delete bptr;
					throw;
				}
			}
			static_cast<virtbuffdet::seppekuer*>(ptr)->reset(ptr);
			return ptr;
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
		using fits=std::integral_constant<bool,sizeof(T)<=BufferSize
			&&alignof(T)<=Alignment
			&&std::is_nothrow_move_constructible<T>::value>;

		void destruct()
		{
			if(_data)
			{
				_data->~Base();
			}
		}
	public:
		virtual_buffer() noexcept:_data{}
		{}
		virtual_buffer(virtual_buffer&& other) noexcept:_data{move(other)}
		{
		}
		virtual_buffer& operator=(virtual_buffer&& other) noexcept(std::is_nothrow_destructible<Base>::value)
		{
			destruct();
			_data=move(other);
			return *this;
		}
		template<std::size_t S,std::size_t A,typename=typename std::enable_if<S<=BufferSize&&A<=Alignment>::type>
		virtual_buffer(virtual_buffer<Base,S,A>&& other) noexcept(std::is_nothrow_destructible<Base>::value)
		{
			_data=move(other);
		}
		template<std::size_t S,std::size_t A>
		auto operator=(virtual_buffer<Base,S,A>&& other) noexcept(std::is_nothrow_destructible<Base>::value) -> typename std::enable_if<S<=BufferSize&&A<=Alignment,virtual_buffer&>::type
		{
			destruct();
			_data=move(other);
			return *this;
		}
		//if your type is final and it does not fit in the buffer, you must provide a specialization for self_deleting_derived
		template<typename T,typename RCT=virtbuffdet::remove_cvref_t<T>,typename=typename std::enable_if<std::is_convertible<RCT*,Base*>::value>::type>
		virtual_buffer(T&& derived_type):virtual_buffer(in_place_type_t<RCT>{},std::forward<T>(derived_type)) {}
		template<typename T,typename... Args>
		virtual_buffer(in_place_type_t<T>,Args&& ... args):_data{construct<T>(
				fits<T>{},
				std::forward<Args>(args)...);}
		{}

		template<typename T,typename U,typename... Args>
		virtual_buffer(in_place_type_t<T> t,std::initializer_list<U> list,Args&&... args):virtual_buffer(t,list,std::forward<Args>(args)...)
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
			destruct();
			try
			{
				_data=construct<T>(fits<T>{},std::forward<Args>(args)...);
			}
			catch(...)
			{
				_data=nullptr;
				throw;
			}
		}

		template<typename T,typename U,typename... Args>
		void emplace(std::initializer_list<U> list,Args&&... args)
		{
			emplace<T>(list,std::forward<Args>(args)...);
		}

		void reset()  noexcept(std::is_nothrow_destructible<Base>::value)
		{
			destruct();
			_data=nullptr;
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