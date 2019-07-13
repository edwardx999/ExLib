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
#ifndef EX_MEM_H
#define EX_MEM_H
#ifndef EXMEM_H
#define EXMEM_H
#include <stdexcept>
#include <vector>
#include <type_traits>
#include <array>
#include <algorithm>
#include <cstdlib>
#include <initializer_list>
#include "extags.h"
#define FORCEINLINE __forceinline
#if (__cplusplus>=201700l)
#define IFCONSTEXPR constexpr
#else
#define IFCONSTEXPR
#endif
namespace exlib {

	template<typename T>
	struct dealloc_uses_size:std::true_type{};

	template<typename T>
	struct dealloc_uses_size<std::allocator<T>>:std::false_type{};

	/*
		A curiously recurring template pattern base for iterators.
		T is value_type of iterator.
	*/
	template<typename T,typename Derived>
	struct iterator_base {
		using value_type=typename std::remove_cv<T>::type;
		using difference_type=std::ptrdiff_t;
		using pointer=T*;
		using reference=T&;
		using iterator_category=std::random_access_iterator_tag;
	private:
		T* _base;
		constexpr Derived& chain()
		{
			return *static_cast<Derived*>(this);
		}
	public:
		//for use by the MSVC stl
		constexpr T* _Unwrapped() const
		{
			return _base;
		}
		constexpr T* base()
		{
			return _base;
		}
		constexpr iterator_base()
		{}
		constexpr iterator_base(T* base):_base(base)
		{}
		constexpr iterator_base(Derived const& other):_base(other.base())
		{}
		constexpr Derived& operator=(Derived const& other)
		{
			_base=other.base();
			return chain();
		}
		constexpr T& operator*() const
		{
			return *_base;
		}
		constexpr T* operator->() const
		{
			return _base;
		}
		constexpr Derived operator++(int)
		{
			return Derived(_base++);
		}
		constexpr Derived& operator++()
		{
			++_base;
			return chain();
		}
		constexpr Derived operator--(int)
		{
			return Derived(_base--);
		}
		constexpr Derived& operator--()
		{
			--_base;
			return chain();
		}
		constexpr std::ptrdiff_t operator-(Derived other) const
		{
			return _base-other.base();
		}
		constexpr Derived operator+(std::ptrdiff_t s) const
		{
			return Derived(_base+s);
		}
		constexpr Derived operator-(std::ptrdiff_t s) const
		{
			return Derived(_base-s);
		}
		constexpr Derived& operator+=(std::ptrdiff_t s)
		{
			_base+=s;
			return chain();
		}
		constexpr Derived& operator-=(std::ptrdiff_t s)
		{
			_base-=s;
			return chain();
		}
		constexpr T& operator[](size_t s) const
		{
			return _base[s];
		}
#define comp(op) constexpr bool operator##op##(Derived other) const { return _base ## op ## other.base() ;}
		comp(<)
			comp(>)
			comp(==)
			comp(>=)
			comp(<=)
			comp(!=)
#undef comp
	};

	/*
		A curiously recurring template pattern base for reverse_iterators.
		T is value_type of iterator.
	*/
	template<typename T,typename Derived>
	struct riterator_base {
		using value_type=T;
		using difference_type=std::ptrdiff_t;
		using pointer=T*;
		using reference=T&;
		using iterator_category=std::random_access_iterator_tag;
	private:
		T* _base;
		constexpr Derived& chain()
		{
			return *static_cast<Derived*>(this);
		}
	public:
		constexpr T* base()
		{
			return _base;
		}
		constexpr riterator_base()
		{}
		constexpr riterator_base(T* base):_base(base)
		{}
		constexpr riterator_base(Derived const& base):_base(other.base())
		{}
		constexpr Derived& operator=(Derived const&)
		{
			_base=other.base();
			return chain();
		}
		constexpr T& operator*() const
		{
			return *(_base-1);
		}
		constexpr T* operator->() const
		{
			return _base-1;
		}
		constexpr Derived operator--(int)
		{
			return Derived(_base++);
		}
		constexpr Derived& operator--()
		{
			++_base;
			return chain();
		}
		constexpr Derived operator++(int)
		{
			return Derived(_base--);
		}
		constexpr Derived& operator++()
		{
			--_base;
			return chain();
		}
		constexpr std::ptrdiff_t operator-(Derived other) const
		{
			return other.base()-_base;
		}
		constexpr Derived operator+(std::ptrdiff_t s) const
		{
			return Derived(_base-s);
		}
		constexpr Derived operator-(std::ptrdiff_t s) const
		{
			return Derived(_base+s);
		}
		constexpr Derived& operator+=(std::ptrdiff_t s)
		{
			_base-=s;
			return chain();
		}
		constexpr Derived& operator-=(std::ptrdiff_t s)
		{
			_base+=s;
			return chain();
		}
		constexpr T& operator[](size_t s) const
		{
			return *(_base-s-1);
		}
#define comp(op) constexpr bool operator##op##(Derived other){ return other.base() ## op ## _base ;}
		comp(<)
			comp(>)
			comp(==)
			comp(>=)
			comp(<=)
			comp(!=)
#undef comp
	};

	template<typename T>
	struct const_iterator:iterator_base<T const,const_iterator<T>> {
		using iterator_base::iterator_base;
	};
	template<typename T>
	struct iterator:iterator_base<T,iterator<T>> {
		using iterator_base::iterator_base;
		iterator(const_iterator<T> ci):iterator_base(ci.base()) {}
	};

	template<typename T>
	struct const_reverse_iterator:iterator_base<T const,const_reverse_iterator<T>> {
		using riterator_base::riterator_base;
	};
	template<typename T>
	struct reverse_iterator:riterator_base<T,reverse_iterator<T>> {
		using riterator_base::riterator_base;
		reverse_iterator(const_reverse_iterator<T> cri):riterator_base(cri.base()) {}
	};

	/*
		A reference to an uninitialized object.
		Is converted to a standard reference when assigned to.
	*/
	template<typename T>
	class uninitialized_reference {
		T* _base;
	public:
		using type=T;
		uninitialized_reference(T& base):_base(std::addressof(base)) {}
		T& operator=(T const& t) noexcept(noexcept(T(t)))
		{
			new (_base)(t);
			return *_base;
		}
		T& operator=(T&& t) noexcept(noexcept(T(std::move(t))))
		{
			new (_base)(std::move(t));
			return *_base;
		}
		template<typename... Args>
		T& emplace(Args&&... args)
		{
			new (_base)(std::forward<Args>(args)...);
			return *_base;
		}
		explicit operator T&() const noexcept
		{
			return *_base;
		}
		uninitialized_reference& operator=(uninitialized_reference const&) noexcept=default;
		uninitialized_reference& operator=(uninitialized_reference&&) noexcept=default;
		void swap(uninitialized_reference& other)
		{
			std::swap(_base,other._base);
		}
		friend void swap(uninitialized_reference& a,uninitialized_reference& b)
		{
			a.swap(b);
		}
	};

	template<typename T>
	class uninitialized_iterator:public iterator_base<T,uninitialized_iterator<T>> {
	public:
		using value_type=T;
		using pointer=T*;
		using reference=uninitialized_reference<T>;
		using iterator_base<T,uninitialized_iterator<T>>::iterator_base;
		constexpr uninitialized_reference<T> operator*() const
		{
			return *base();
		}
	private:
		constexpr T* operator->() const
		{
			return base();
		}
	public:
		constexpr uninitialized_reference<T> operator[](size_t s) const
		{
			return base()[s];
		}
	};

	template<typename T,typename Base=std::allocator<T>>
	class default_init_allocator:public Base {
	public:
		using Base::Base;
		void construct(T* place)
		{
			new (place) T;
		}
		using Base::construct;
	};

	template<typename T,typename Base=std::allocator<T>>
	class no_init_allocator:public Base {
	public:
		using Base::Base;
		template<typename... Args>
		void construct(T* place,Args&&... args)
		{}
	};

	namespace detail {

		template<typename... Types>
		struct type_pack;

		template<>
		struct type_pack<> {};

		template<typename T>
		struct type_pack<T> {
			using type=T;
		};

		template<typename T,typename... U>
		struct type_pack<T,U...> {
			using type=T;
			using next=type_pack<U...>;
		};

		template<typename T>
		struct type_pack_size;

		template<>
		struct type_pack_size<type_pack<>>:std::integral_constant<size_t,0> {};

		template<typename T>
		struct type_pack_size<type_pack<T>>:std::integral_constant<size_t,sizeof(T)> {};

		template<typename T,typename... Rest>
		struct type_pack_size<type_pack<T,Rest...>>:
			std::integral_constant<size_t,
			sizeof(T)+
			type_pack_size<type_pack<Rest...>>::value> {};

		template<typename T>
		struct type_pack_count;

		template<typename... T>
		struct type_pack_count<type_pack<T...>>:std::integral_constant<size_t,sizeof...(T)> {};

		template<size_t Idx,typename TypePack>
		struct type_pack_get;

		template<typename First,typename... Rest>
		struct type_pack_get<0,type_pack<First,Rest...>> {
			using type=First;
		};

		template<size_t Idx,typename First,typename... Rest>
		struct type_pack_get<Idx,type_pack<First,Rest...>>:type_pack_get<Idx-1,type_pack<Rest...>> {};

		template<size_t N>
		struct mvector_to_alloc {
			char d[N];
		};

		template<typename TypePack,template<typename> typename Allocator=std::allocator>
		class mvector:protected Allocator<mvector_to_alloc<type_pack_size<TypePack>::value>> {
		protected:
			template<size_t I>
			using get_t=typename type_pack_get<I,TypePack>::type;
		public:
			static constexpr size_t type_count=type_pack_count<TypePack>::value;
		private:
			template<size_t I,size_t... Is>
			static constexpr size_t get_alignment(std::index_sequence<I,Is...>)
			{
				return std::max(alignof(get_t<I>),get_alignment(std::index_sequence<Is...>()));
			}
			static constexpr size_t get_alignment(std::index_sequence<>)
			{
				return 0;
			}
		protected:
			static constexpr size_t alignment=get_alignment(std::make_index_sequence<type_count>());
		public:
			template<size_t I>
			using value_type=get_t<I>;
			template<size_t I>
			using reference=get_t<I>&;
			template<size_t I>
			using const_reference=get_t<I> const&;
			template<size_t I>
			using pointer=get_t<I>*;
			template<size_t I>
			using const_pointer=get_t<I> const*;

			template<size_t I>
			struct const_iterator:iterator_base<get_t<I> const,const_iterator<I>> {
				using iterator_base::iterator_base;
			};
			template<size_t I>
			struct iterator:iterator_base<get_t<I>,iterator<I>> {
				using iterator_base::iterator_base;
			};

			template<size_t I>
			struct reverse_iterator:riterator_base<get_t<I>,reverse_iterator<I>> {
				using riterator_base::riterator_base;
			};
			template<size_t I>
			struct const_reverse_iterator:riterator_base<get_t<I> const,const_reverse_iterator<I>> {
				using riterator_base::riterator_base;
			};

			using size_type=size_t;
			using difference_type=std::ptrdiff_t;

		protected:
			using _AllocType=mvector_to_alloc<type_pack_size<TypePack>::value>;
			using _Alloc=Allocator<_AllocType>;
		public:
			using allocator_type=_Alloc;
		protected:
			char* do_alloc(size_t amount)
			{
				return reinterpret_cast<char*>(_Alloc::allocate(amount));
			}
			void do_dealloc(char* what)
			{
				_Alloc::deallocate(reinterpret_cast<_AllocType*>(what),_cap);
			}
			size_t _cap;
			char* _data;
			size_t _size;
		private:
			template<size_t B,size_t E>
			struct size_up_to_h:std::integral_constant<size_t,sizeof(get_t<B>)+size_up_to_h<B+1,E>::value> {};

			template<size_t N>
			struct size_up_to_h<N,N>:std::integral_constant<size_t,0> {};

			template<size_t I>
			using size_up_to=size_up_to_h<0,I>;
		protected:
			template<size_t I>
			size_t type_offset() const
			{
				return size_up_to<I>::value*_cap;
			}
		public:
			template<size_t I=0>
			iterator<I> begin() noexcept
			{
				return reinterpret_cast<get_t<I>*>(_data+type_offset<I>());
			}
			template<size_t I=0>
			const_iterator<I> begin() const noexcept
			{
				return reinterpret_cast<get_t<I> const*>(_data+type_offset<I>());
			}
			template<size_t I=0>
			const_iterator<I> cbegin() const noexcept
			{
				return begin<I>();
			}
			template<size_t I=0>
			iterator<I> end() noexcept
			{
				return reinterpret_cast<get_t<I>*>(_data+type_offset<I>())+_size;
			}
			template<size_t I=0>
			const_iterator<I> end() const noexcept
			{
				return reinterpret_cast<get_t<I> const*>(_data+type_offset<I>())+_size;
			}
			template<size_t I=0>
			const_iterator<I> cend() const noexcept
			{
				return end<I>();
			}

			template<size_t I=0>
			reverse_iterator<I> rbegin() noexcept
			{
				return reinterpret_cast<get_t<I>*>(_data+type_offset<I>())+_size;
			}
			template<size_t I=0>
			const_reverse_iterator<I> rbegin() const noexcept
			{
				return reinterpret_cast<get_t<I> const*>(_data+type_offset<I>())+_size;
			}
			template<size_t I=0>
			const_reverse_iterator<I> crbegin() const noexcept
			{
				return rbegin<I>();
			}
			template<size_t I=0>
			reverse_iterator<I> rend() noexcept
			{
				return reinterpret_cast<get_t<I>*>(_data+type_offset<I>());
			}
			template<size_t I=0>
			const_reverse_iterator<I> rend() const noexcept
			{
				return reinterpret_cast<get_t<I> const*>(_data+type_offset<I>());
			}
			template<size_t I=0>
			const_reverse_iterator<I> crend() const noexcept
			{
				return rend<I>();
			}
		protected:
			template<size_t I>
			void move_range(char* new_buffer,size_t new_cap)
			{
				using type=get_t<I>;
				constexpr auto offset_unit=size_up_to<I>::value;
				type* old=reinterpret_cast<type*>(_data+_cap*offset_unit);
				type* nb=reinterpret_cast<type*>(new_buffer+new_cap*offset_unit);
				std::memcpy(nb,old,_size*sizeof(type));
				if IFCONSTEXPR(!std::is_trivially_destructible<type>::value)
				{
					for(size_t i=0;i<_size;++i)
					{
						old[i].~type();
					}
				}
			}

			void move_ranges(char* new_buffer,size_t new_cap,std::index_sequence<>)
			{}
			template<size_t I,size_t... Is>
			void move_ranges(char* new_buffer,size_t new_cap,std::index_sequence<I,Is...>)
			{
				move_range<I>(new_buffer,new_cap);
				move_ranges(new_buffer,new_cap,std::index_sequence<Is...>());
			}

			template<size_t I>
			void copy_range(char const* src,size_t src_offset)
			{
				using type=get_t<I>;
				constexpr size_t offset=size_up_to<I>::value;
				type* odst=reinterpret_cast<type*>(_data+offset*_cap);
				type const* osrc=reinterpret_cast<type const*>(src+offset*src_offset);
				for(size_t i=0;i<_size;++i)
				{
					new (odst+i) type(osrc[i]);
				}
			}
			template<size_t I,size_t... Is>
			void copy_range(char const* src,size_t src_offset,std::index_sequence<I,Is...>)
			{
				copy_range<I>(src,src_offset);
				copy_range(src,src_offset,std::index_sequence<Is...>());
			}
			void copy_range(char const* src,size_t src_offset,std::index_sequence<>)
			{}

			void realloc(size_t new_cap)
			{
				assert(new_cap%alignment==0);
				char* temp=do_alloc(new_cap);
				move_ranges(temp,new_cap,std::make_index_sequence<type_count>());
				do_dealloc(_data);
				_cap=new_cap;
				_data=temp;
			}
		public:
			template<size_t I=0>
			get_t<I> const* data() const noexcept
			{
				return reinterpret_cast<get_t<I> const*>(_data+type_offset<I>());
			}
			template<size_t I=0>
			get_t<I>* data() noexcept
			{
				return reinterpret_cast<get_t<I>*>(_data+type_offset<I>());
			}
			bool empty() const noexcept
			{
				return size==0;
			}
			size_t size() const noexcept
			{
				return _size;
			}
			size_t capacity() const noexcept
			{
				return _cap;
			}
			static constexpr size_t max_size() noexcept
			{
				return std::numeric_limits<size_t>::max()/sizeof(_AllocType);
			}
			mvector(mvector const& other):_cap(other._cap),_data(do_alloc(_cap)),_size(other._size)
			{
				copy_range(other._data,other._cap,std::make_index_sequence<type_count>());
			}
			mvector(mvector&& other):_cap(other._cap),_data(other._data),_size(other._size)
			{
				other._data=nullptr;
				other._size=0;
				other._cap=0;
			}
			mvector():_cap(0),_data(nullptr),_size(0)
			{}
		private:
			static constexpr size_t fix_alignment(size_t s)
			{
				return s+s%alignment;
			}

			template<size_t I,size_t E>
			struct constructor {
				template<typename... Rest>
				static void construct(size_t begin,size_t end,char* data,size_t cap,get_t<I>&& val,Rest&&... rest)
				{
					get_t<I>* o=reinterpret_cast<get_t<I>*>(data+size_up_to<I>::value*cap);
					for(size_t i=begin;i<end;++i)
					{
						new (o+i) get_t<I>(std::forward<get_t<I>>(val));
					}
					constructor<I+1,E>::construct(begin,end,data,cap,std::forward<Rest>(rest)...);
				}
				static void construct(size_t begin,size_t end,char* data,size_t cap)
				{
					get_t<I>* o=reinterpret_cast<get_t<I>*>(data+size_up_to<I>::value*cap);
					if IFCONSTEXPR(!std::is_trivially_constructible<get_t<I>>::value)
					{
						for(size_t i=begin;i<end;++i)
						{
							new (o+i) get_t<I>;
						}
					}
					constructor<I+1,E>::construct(begin,end,data,cap);
				}
			};
			template<size_t I>
			struct constructor<I,I> {
				static void construct(size_t,size_t s,char* data,size_t cap)
				{}
			};

		public:
			template<typename... Args>
			mvector(size_t s,Args const&... args):_cap(fix_alignment(s)),_data(do_alloc(_cap)),_size(s)
			{
				static_assert(sizeof...(Args)<=type_count);
				constructor<0,type_count>::construct(s,_data,_cap,args);
			}
			void reserve(size_t s)
			{
				if(s>_cap)
				{
					realloc(fix_alignment(s));
				}
			}
			void shrink_to_fit()
			{
				realloc(fix_alignment(_size));
			}
			template<typename... Args>
			void push_back(Args&&... args)
			{
				static_assert(sizeof...(args)<=type_count,"Too many arguments");
				size_t new_size=_size+1;
				if(new_size>_cap)
				{
					realloc(2*_cap+alignment);
				}
				constructor<0,type_count>::construct(_size,new_size,_data,_cap,std::forward<Args>(args)...);
				_size=new_size;
			}

		protected:
			template<size_t B,size_t E>
			struct inserter {
				template<typename First,typename... Args>
				static void insert(size_t pos,size_t remaining,size_t count,char* base,size_t cap,First&& f,Args&&... args)
				{
					size_t offset=size_up_to<B>::value*cap;
					using Type=get_t<B>;
					Type* data=reinterpret_cast<Type*>(base+offset)+pos;
					std::memmove(data+count,data,remaining*sizeof(Type));
					for(size_t i=0;i<count;++i)
					{
						new (data+i) Type(std::forward<First>(f));
					}
					inserter<B+1,E>::insert(pos,remaining,count,base,cap,std::forward<Args>(args)...);
				}
				static void insert(size_t pos,size_t remaining,size_t count,char* base,size_t cap)
				{
					size_t offset=size_up_to<B>::value*cap;
					using Type=get_t<B>;
					Type* data=reinterpret_cast<Type*>(base+offset)+pos;
					std::memmove(data+count,data,remaining*sizeof(Type));
					if IFCONSTEXPR(!std::is_trivially_constructible<Type>::value)
					{
						for(size_t i=0;i<count;++i)
						{
							new (data+i) Type;
						}
					}
					inserter<B+1,E>::insert(pos,remaining,count,base,cap);
				}
			};
			template<size_t B>
			struct inserter<B,B> {
				static void insert(size_t,size_t,size_t,char*,size_t)
				{}
			};

			template<size_t I,typename Iter,typename... Args>
			void insert_impl(Iter pos,size_t count,Args&&... args)
			{
				size_t remaining=end<I>()-pos;
				size_t off=pos-begin<I>();
				size_t new_size=_size+count;
				if(new_size>_cap)
				{
					realloc(fix_alignment(new_size));
				}
				_size=new_size;
				inserter<0,type_count>::insert(off,remaining,count,_data,_cap,std::forward<Args>(args)...);
			}
		public:
			template<size_t I,typename... Args>
			void insert_n(iterator<I> pos,size_t count,Args const&... args)
			{
				insert_impl<I>(pos,count,args...);
			}
			template<size_t I,typename... Args>
			void insert_n(const_iterator<I> pos,size_t count,Args const&... args)
			{
				insert_impl<I>(pos,count,args...);
			}
			template<size_t I,typename... Args>
			void insert(iterator<I> pos,Args&&... args)
			{
				insert_impl<I>(pos,1,std::forward<Args>(args)...);
			}
			template<size_t I,typename... Args>
			void insert(const_iterator<I> pos,Args&&... args)
			{
				insert_impl<I>(pos,1,std::forward<Args>(args)...);
			}

		private:
			template<size_t B,size_t E>
			struct eraser {
				static void erase(size_t pos,size_t remaining,size_t count,char* data,size_t cap)
				{
					using T=get_t<B>;
					T* o=reinterpret_cast<T*>(data+size_up_to<B>::value*cap)+pos;
					if IFCONSTEXPR(!std::is_trivially_destructible<T>::value)
					{
						for(size_t i=0;pos<count;++i)
						{
							o[i].~T();
						}
					}
					std::memmove(o,o+count,remaining*sizeof(T));
					eraser<B+1,E>::erase(pos,remaining,count,data,cap);
				}
			};
			template<size_t E>
			struct eraser<E,E> {
				static void erase(size_t,size_t,size_t,char*,size_t)
				{}
			};
			template<size_t I,typename Iter>
			void erase_impl(Iter first,Iter last)
			{
				size_t remaining=end<I>()-last;
				size_t count=last-first;
				size_t pos=first-begin<I>();
				eraser<0,type_count>::erase(pos,remaining,count,_data,_cap);
				_size-=count;
			}
		public:
			template<size_t I>
			void erase(iterator<I> first,iterator<I> last)
			{
				erase_impl<I>(first,last);
			}
			template<size_t I>
			void erase(const_iterator<I> first,const_iterator<I> last)
			{
				erase_impl<I>(first,last);
			}
			template<size_t I>
			void erase(iterator<I> first)
			{
				erase_impl<I>(first,first+1);
			}
			template<size_t I>
			void erase(const_iterator<I> first)
			{
				erase_impl<I>(first,first+1);
			}
			void pop_back()
			{
				erase(end()-1);
			}
			void clear()
			{
				erase(begin(),end());
			}
		private:
			template<size_t... Is>
			void resize_grow(size_t s,std::index_sequence<Is...>)
			{
				insert_n(end(),s-_size);
			}
		public:
			void resize(size_t s)
			{
				if(s<_size)
				{
					erase(begin()+s,end());
				}
				if(s>_size)
				{
					resize_grow(s,std::make_index_sequence<type_count>());
				}
			}
			mvector& operator=(mvector const& other)
			{
				clear();
				reserve(other._size);
				copy_range(other._data,other._cap,std::make_index_sequence<type_count>());
				return *this;
			}
			~mvector()
			{
				eraser<0,type_count>::erase(0,0,_size,_data,_cap);
				do_dealloc(_data);
			}
			mvector& operator=(mvector&& other)
			{
				eraser<0,type_count>::erase(0,0,_size,_data,_cap);
				do_dealloc(_data);
				_data=other._data;
				_cap=other._cap;
				_size=other._size;
				other._data=nullptr;
				other._size=0;
				other._cap=0;
				return *this;
			}
		};
	}

	//Creates a vector that organizes the given types 
	//as an array of type1 next to array of type2 next to array of type3...
	//all of the same size
	template<typename Type1,typename... Types>
	using multi_vector=detail::mvector<detail::type_pack<Type1,Types...>>;

	template<typename T,std::size_t size>
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

	//A buffer to hold a virtual object using small object optimization
	template<typename Base,std::size_t buffer_size=sizeof(Base),std::size_t alignment=alignof(Base)>
	class virtual_buffer;
	//A buffer that might hold a virtual object using small object optimization
	template<typename Base,std::size_t buffer_size=sizeof(Base),std::size_t alignment=alignof(Base)>
	class nullable_virtual_buffer;

	namespace virtual_buffer_detail {
		template<bool val>
		using bool_constant=std::integral_constant<bool,val>;
		struct self_freer {
			void* _location;
		public:
			self_freer(char* location) noexcept:_location{location} {}
			~self_freer() noexcept
			{
				std::free(_location);
			}
		};
		template<typename Base,typename Allocator>
		class seppukuer:self_freer,public Base  {
		public:
			template<typename... Args>
			seppukuer(void* location,Args&&... args):self_freer{location,sizeof(seppukuer)},Base(std::forward<Args>(args)...) {}
			~seppukuer() override = default;
		};
		template<typename Base,std::size_t buffer_size,std::size_t alignment,typename Derived>
		class virtual_buffer_base {
			static_assert(std::has_virtual_destructor<Base>::value,"Virtual destructor required");
			friend class exlib::virtual_buffer<Base,buffer_size,alignment>;
			friend class exlib::nullable_virtual_buffer<Base,buffer_size,alignment>;
			typename std::aligned_storage<buffer_size,alignment>::type _data;
			Base* _object;
			template<typename T,typename... Args>
			static void construct(std::true_type,void* location,Args&&... args) noexcept
			{
				new (data) T(data, std::forward<Args>(args)...);
			}
			template<typename T,typename... Args>
			static void construct(std::false_type, void* location, Args&& ... args)
			{
				try
				{
					new (data) T(data,std::forward<Args>(args)...);
				}
				catch (...)
				{
					free(data);
					throw;
				}
			}
			template<typename T,typename... Args>
			void init_help(std::false_type,Args&&... args)
			{
				using AllocType=seppukuer<T>;
				static_assert(alignof(T)<=alignof(std::max_align_t), "No overalignment allowed");
				char* const data=std::malloc(sizeof(AllocType));
				if (data)
				{
					construct<AllocType>(bool_constant<std::is_nothrow_constructible<T,Args&&...>{},std::forward<Args>(args)...);
					_object=static_cast<Base*>(reinterpret_cast<AllocType*>(data));
				}
				else
				{
					throw std::bad_alloc{};
				}
			}
			template<typename T,typename... Args>
			void init_help(std::true_type,Args&&... args)
			{
				new (&_data) T(std::forward<Args>(args)...);
				_object=static_cast<Base*>(reinterpret_cast<T*>(&_data));
			}
			template<typename Type>
			using fits=typename Derived::fits;
			template<typename T,typename... Args>
			void init(Args&&... args)
			{
				init_help<T>(fits<T>{},std::forward<Args>(args)...);
			}
		public:
			template<typename Child,typename... Args>
			virtual_buffer_base(exlib::in_place_type_t<Child>,Args&&... args) noexcept(std::is_nothrow_constructible<Child,Args&&...>::value&&fits<Child>::value)
			{
				init<Child>(std::forward<Args>(args)...);
			}
			template<typename Child,typename U,typename... Args>
			virtual_buffer_base(exlib::in_place_type_t<Child>,std::initializer_list<U> list,Args&&... args) noexcept(std::is_nothrow_constructible<Child,std::initializer_list<U>,Args&&...>::value&& fits<Child>::value)
			{
				init<Child>(list,std::forward<Args>(args)...);
			}
			template<typename T,typename... Args>
			auto emplace(Args&&... args) -> typename std::enable_if<std::is_convertible<T*, Base*>::value, decltype(T(std::forward<Args>(args)...),*_object)>::type
			{
				static_cast<Derived&>(*this).reset();
				init<T>(std::forward<Args>(args)...);
				return *_object;
			}
			Base* operator->()
			{
				return _object;
			}
			Base& operator*()
			{
				return *_object;
			}
			Base const* operator->() const
			{
				return _object;
			}
			Base const& operator*() const
			{
				return *_object;
			}
		};
	}

	template<typename Base,std::size_t buffer_size,std::size_t alignment>
	class virtual_buffer:public virtual_buffer_detail::virtual_buffer_base<Base,buffer_size,alignment,virtual_buffer<Base,buffer_size,alignment>> {
		using Super=virtual_buffer_detail::virtual_buffer_base<Base,buffer_size,alignment,virtual_buffer<Base,buffer_size,alignment>>;
	public:
		template<typename Type>
		struct fits:bool_constant<sizeof(Type)<=decltype(_data)&&alignof(Type)<=decltype(_data)>{};
		using Super::Super;
		template<typename... Args>
		virtual_buffer(Args&&... args) noexcept(std::is_nothrow_constructible<Base,Args&&...>::value)
		{
			Super::init<Base>(std::forward<Args>(args)...);
		}
		template<typename U,typename... Args>
		virtual_buffer(std::initializer_list<U> list,Args&&... args) noexcept(std::is_nothrow_constructible<Base,std::initializer_list<U>,Args&&...>::value)
		{
			Super::init<Base>(list,std::forward<Args>(args)...);
		}
		virtual_buffer(virtual_buffer&)=delete;
		virtual_buffer(virtual_buffer const&)=delete;
		virtual_buffer(virtual_buffer&&)=delete;
		virtual_buffer(virtual_buffer const&&)=delete;
		void reset() noexcept(noexcept(this->_object->~Base()))
		{
			this->_object->~Base();
		}
		~virtual_buffer() noexcept(noexcept(reset()))
		{
			reset();
		}
	};
	template<typename Base,std::size_t buffer_size,std::size_t alignment>
	class nullable_virtual_buffer:public virtual_buffer_detail::virtual_buffer_base<Base,buffer_size,alignment,nullable_virtual_buffer<Base,buffer_size,alignment>> {
		using Super=virtual_buffer_detail::virtual_buffer_base<Base,buffer_size,alignment,nullable_virtual_buffer<Base,buffer_size,alignment>>;
	public:
		template<typename Type>
		struct fits:bool_constant<sizeof(Type)<=decltype(_data)&&alignof(Type)<=decltype(_data)&&std::is_nothrow_move_constructible<Type>::value>{};
		nullable_virtual_buffer() noexcept
		{
			this->_object=nullptr;
		}
		using Super::Super;
		
		void reset() noexcept(noexcept(this->_object->~Base()))
		{
			if(this->_object)
			{
				this->_object->~Base();
				this->_object=nullptr;
			}
		}

		~nullable_virtual_buffer() noexcept(noexcept(reset()))
		{
			if(this->_object)
			{
				this->_object->~Base();
			}
		}
		
		nullable_virtual_buffer(nullable_virtual_buffer&& other) noexcept
		{
			auto const oobj=reinterpret_cast<std::size_t>(other._object);
			auto const lo=reinterpret_cast<std::size_t>(&other._data);
			auto const up=reinterpret_cast<std::size_t>(&other._data+1);
			if(oobj>=lo&&oobj<up)
			{
				std::memcpy(&this->_data,&other._data,sizeof(this->_data));
				this->_object=reinterpret_cast<Base*>(reinterpret_cast<char*>(&this->_data)+oobj-lo);
			}
			else
			{
				this->_object=other._object;
			}
		}
		nullable_virtual_buffer& operator=(nullable_virtual_buffer&& other) noexcept
		{
			this->~nullable_virtual_buffer();
			new (this) nullable_virtual_buffer(std::move(other));
			return *this;
		}
	};
}
#undef IFCONSTEXPR
#endif
#endif