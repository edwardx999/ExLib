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
#include <stdexcept>
#include <vector>
#include <type_traits>
#include <array>
#include <algorithm>
#ifdef _MSVC_LANG
#define EXMEM_HAS_CPP_17 _MSVC_LANG>201700L
#else
#define EXMEM_HAS_CPP_17 _cplusplus>201700L
#endif
#ifdef _MSVC_LANG
#define EXMEM_HAS_CPP_20 _MSVC_LANG>202000L
#else
#define EXMEM_HAS_CPP_20 _cplusplus>202000L
#endif
#define FORCEINLINE __forceinline
#if EXMEM_HAS_CPP_17
#define IFCONSTEXPR constexpr
#else
#define IFCONSTEXPR
#endif
#include "exiterator.h"
namespace exlib {

	//A smart ptr that uses the given allocator to allocate and delete
	//memory is uninitilized, and you are responsible for destroy any constructed objects
	template<typename T,typename Allocator>
	class allocator_ptr:private Allocator {
	private:
		using Traits=std::allocator_traits<Allocator>;
	public:
		using pointer=Traits::pointer;
		using element_type=Traits::value_type;
		using allocator_type=Allocator;
		using reference=Traits::reference;
	private:
		pointer _base;
		size_t _capacity;
	public:
		void release()
		{
			_base=nullptr;
		}
		void reset()
		{
			Traits::deallocate(_base,_capacity);
			_base=nullptr;
		}
		void reset(pointer p,size_t capacity)
		{
			Traits::deallocate(_base,_capacity);
			_base=p;
			_capacity=capacity;
		}
		allocator_ptr(size_t n,Allocator const& alloc=Allocator()):Allocator(alloc),_base(Allocator::allocate(n)),_capacity(n)
		{}
		allocator_ptr(Allocator const& alloc=Allocator()):Allocator(alloc),_base(nullptr),_capacity(0)
		{}
		allocator_ptr(allocator_ptr&& o):Allocator(std::move(o)),_base(o._base),_capacity(o._capacity)
		{
			o.release();
		}
		size_t capacity() const
		{
			return _capacity;
		}
		pointer get() const
		{
			return _base;
		}
		void swap(allocator_ptr& o)
		{
			using std::swap;
			swap(_base,o._base);
			swap(_capacity,o._capacity);
		}
		friend void swap(allocator_ptr& a,allocator_ptr& b)
		{
			a.swap(b);
		}
		allocator& operator=(allocator_ptr&& o)
		{
			reset(o._base,o._capacity);
			o.release();
			return *this;
		}
		~allocator_ptr()
		{
			Traits::deallocate(_base,_capacity);
		}
		reference operator[](size_t s) const
		{
			return _base[s];
		}
		reference operator*() const
		{
			return *_base;
		}
		Allocator get_allocator() const
		{
			return *this;
		}
	};

	namespace detail {

		template<typename T>
		struct tuple_size_sum;
#if	EXMEM_HAS_CPP_17

		template<typename...Types>
		struct tuple_size_sum<std::tuple<Types...>>:std::integral_constant<size_t,(...+sizeof(Types))> {};
#else
		template<>
		struct tuple_size_sum<std::tuple<>>:std::integral_constant<size_t,0> {};
		template<typename T>
		struct tuple_size_sum<std::tuple<T>>:std::integral_constant<size_t,sizeof(T)> {};

		template<typename First,typename... Rest>
		struct tuple_size_sum<std::tuple<First,Rest...>>:std::integral_constant<size_t,sizeof(First)+tuple_size_sum<Rest...>::value> {};
#endif
		template<size_t N>
		class mvector_to_alloc {
			char data[N];
		};
	}

	template<typename... Types>
	struct types:std::tuple<Types...> {
		using types=std::tuple<Types...>;
	};

	template<typename T>
	struct pointerlike_wrapper {
	private:
		T _value;
	public:
		pointerlike_wrapper(T v):_value(std::move(v))
		{}
		T& operator*() const
		{
			return _value;
		}
		T* operator->() const
		{
			return &_value;
		}
	};

	template<typename T>
	auto wrap_pointerlike(T&& arg)->pointerlike_wrapper<typename std::remove_cv<typename std::remove_reference<T>::type>::type>
	{
		return {std::forward<T>(arg)};
	}

	/*
		Add subrange (alias for span) method to iterator each subrange
		Fix constructors to use constructors
	*/
	namespace detail {
		template<typename Types,typename Allocator>
		class mvector;

		template<typename... Types,typename Allocator>
		class mvector<types<Types...>,Allocator> {
		protected:
			using TypeTuple=std::tuple<Types...>;
		public:
			static constexpr size_t type_count=std::tuple_size<TypeTuple>::value;
		private:
			template<size_t I>
			using get_t=typename std::tuple_element<TypeTuple>::type;

			using idx_seq=std::make_index_sequence<type_count>;

			template<size_t I,size_t... Is>
			static constexpr size_t get_alignment(std::index_sequence<I,Is...>)
			{
				return std::max(alignof(get_t<I>),get_alignment(std::index_sequence<Is...>()));
			}
			static constexpr size_t get_alignment(std::index_sequence<>)
			{
				return 1;
			}
			using AllocBase=std::allocator_traits<Allocator>::rebind_alloc<mvector_to_alloc<tuple_size_sum<Types...>::value>>;
			using AllocTraits=std::allocator_traits<AllocBase>;

			template<size_t I>
			using get_construct_t=typename std::tuple_element<ConstructorsTuple>::type;
			template<size_t I>
			using get_construct_traits_t=constructor_traits<get_consstruct_t<I>>;
		protected:
			static constexpr size_t alignment=get_alignment(idx_seq{});
		public:
			template<size_t I>
			using subrange_value_type=get_t<I>;
			template<size_t I>
			using subrange_reference=get_t<I>&;
			template<size_t I>
			using const_subrange_reference=get_t<I> const&;
			template<size_t I>
			using subrange_pointer=get_t<I>*;
			template<size_t I>
			using const_subrange_pointer=get_t<I> const*;

			using const_reference=std::tuple<Types const&...>;
			using reference=std::tuple<Types&...>;
			using value_type=std::tuple<Types&...>;

			class const_iterator;

			class iterator {
			public:
				friend class class_iterator;
				using iterator_category=std::random_access_iterator_tag;
				using value_type=typename mvector::value_type;
				using difference_type=std::ptrdiff_t;
				using pointer=pointerlike_wrapper<value_type>;
				using reference=typename mvector::reference;
				friend class multi_vector;
			private:
				mvector* _parent;
				size_t _index;
			public:
				iterator(mvector& parent,size_t index):_parent(&parent),_index(index)
				{}
				iterator(iterator const&)=default;
				iterator& operator=(iterator const&)=default;
#define iterator_op(op) iterator operator op(size_t s) const {return iterator(*_parent,_index##op##s);} iterator& operator ##op##=(size_t s){return _index##op##=s,*this;} 
				iterator_op(+)
					iterator_op(-)
#undef iterator_op
#define iterator_comp(op) bool operator op(iterator const& o) const {assert(_parent==o._parent);return _index##op##o._index;}
					iterator_comp(<)
					iterator_comp(>)
					iterator_comp(!=)
					iterator_comp(<=)
					iterator_comp(>=)
					iterator_comp(<=)
#undef iterator_comp
					reference operator[](size_t s) const
				{
					return (*_parent)[s];
				}
				reference operator*() const
				{
					return (*_parent)[0];
				}
				reference operator->()const
				{
					return wrap_pointerlike((*_parent)[0]);
				}
#define iterator_ment(op) iterator& operator ##op##() {##op##_index;return *this;} iterator operator ##op##(int) {iterator copy(*this);##op##_index;return copy;} 
				iterator_ment(++)
					iterator_ment(--)
#undef iterator_ment
			};

			class const_iterator {
			public:
				using iterator_category=std::random_access_iterator_tag;
				using value_type=typename mvector::const_reference;
				using difference_type=std::ptrdiff_t;
				using pointer=pointerlike_wrapper<const_reference>;
				using reference=typename mvector::const_reference;
				friend class multi_vector;
			private:
				mvector const* _parent;
				size_t _index;
			public:
				const_iterator(mvector const& parent,size_t index):_parent(&parent),_index(index)
				{}
				const_iterator(iterator const& o):const_iterator(o._parent,o._index)
				{}
				const_iterator(const_iterator const&)=default;
				const_iterator& operator=(const_iterator const&)=default;
#define iterator_op(op) const_iterator operator op(size_t s) const {return const_iterator(*_parent,_index##op##s);} const_iterator& operator ##op##=(size_t s){return _index##op##=s,*this;} 
				iterator_op(+)
					iterator_op(-)
#undef iterator_op
#define iterator_comp(op) bool operator op(const_iterator const& o) const {assert(_parent==o._parent);return _index##op##o._index;}
					iterator_comp(<)
					iterator_comp(>)
					iterator_comp(!=)
					iterator_comp(<=)
					iterator_comp(>=)
					iterator_comp(<=)
#undef iterator_comp
					reference operator[](size_t s) const
				{
					return (*_parent)[s];
				}
				reference operator*() const
				{
					return (*_parent)[0];
				}
				reference operator->()const
				{
					return wrap_pointerlike((*_parent)[0]);
				}
#define iterator_ment(op) const_iterator& operator ##op##() {##op##_index;return *this;} const_iterator operator ##op##(int) {const_iterator copy(*this);##op##_index;return copy;} 
				iterator_ment(++)
					iterator_ment(--)
#undef iterator_ment
			};

			using reverse_iterator=std::reverse_iterator<iterator>;
			using const_reverse_iterator=std::reverse_iterator<const_iterator>;

			template<size_t I>
			struct const_subrange_iterator:iterator_base<get_t<I> const,const_iterator<I>> {
				using iterator_base::iterator_base;
			};
			template<size_t I>
			struct subrange_iterator:iterator_base<get_t<I>,iterator<I>> {
				using iterator_base::iterator_base;
			};

			template<size_t I>
			struct reverse_subrange_iterator:riterator_base<get_t<I>,reverse_iterator<I>> {
				using riterator_base::riterator_base;
			};
			template<size_t I>
			struct const_reverse_subrange_iterator:riterator_base<get_t<I> const,const_reverse_iterator<I>> {
				using riterator_base::riterator_base;
			};

			using size_type=size_t;
			using difference_type=std::ptrdiff_t;

		public:
			using allocator_type=AllocBase;
		protected:
			using _DataM=allocator_ptr<void,allocator_type>;

			template<size_t I,typename... Args>
			void do_construct(get_t<I>* location,Args&&... args)
			{
				AllocTraits::construct(_data.get_allocator(),location,std::forward<Args>(args)...);
			}
			template<size_t I>
			void do_destroy(get_t<I>* location)
			{
				AllocTraits::destroy(_data.get_allocator(),location);
			}

			_DataM _data;
			size_t _size;
		private:
			template<size_t B,size_t E>
			struct size_up_to_h:std::integral_constant<size_t,sizeof(get_t<B>)+size_up_to_h<B+1,E>::value> {};

			template<size_t N>
			struct size_up_to_h<N,N>:std::integral_constant<size_t,0> {};

			template<size_t I>
			using size_up_to=size_up_to_h<0,I>;
		public:
			size_t capacity() const noexcept
			{
				return _data.capacity();
			}
			static constexpr size_t max_size() noexcept
			{
				return std::numeric_limits<size_t>::max()/sizeof(_AllocType);
			}
		protected:
			template<size_t I>
			size_t type_offset() const
			{
				return size_up_to<I>::value*capacity();
			}
		public:

			void* data() noexcept
			{
				return _data.get();
			}
			void const* data() const noexcept
			{
				return _data.get();
			}
			size_t size() const noexcept
			{
				return _size;
			}
			bool empty() const noexcept
			{
				return size==0;
			}

			template<size_t I>
			get_t<I> const* data() const noexcept
			{
				return reinterpret_cast<get_t<I> const*>(static_cast<char*>(data())+type_offset<I>());
			}
			template<size_t I>
			get_t<I>* data() noexcept
			{
				return reinterpret_cast<get_t<I>*>(static_cast<char*>(data())+type_offset<I>());
			}

			template<size_t I>
			subrange_iterator<I> begin() noexcept
			{
				return data<I>();
			}
			template<size_t I>
			const_subrange_iterator<I> begin() const noexcept
			{
				return data<I>();
			}
			template<size_t I>
			const_subrange_iterator<I> cbegin() const noexcept
			{
				return begin<I>();
			}
			template<size_t I>
			subrange_iterator<I> end() noexcept
			{
				return data<I>()+size();
			}
			template<size_t I>
			const_subrange_iterator<I> end() const noexcept
			{
				return data<I>()+size();
			}
			template<size_t I>
			const_subrange_iterator<I> cend() const noexcept
			{
				return end<I>();
			}

			template<size_t I>
			reverse_subrange_iterator<I> rbegin() noexcept
			{
				return end();
			}
			template<size_t I>
			const_reverse_subrange_iterator<I> rbegin() const noexcept
			{
				return end();
			}
			template<size_t I>
			const_reverse_subrange_iterator<I> crbegin() const noexcept
			{
				return end();
			}
			template<size_t I>
			reverse_subrange_iterator<I> rend() noexcept
			{
				return begin();
			}
			template<size_t I>
			const_reverse_subrange_iterator<I> rend() const noexcept
			{
				return begin();
			}
			template<size_t I>
			const_reverse_subrange_iterator<I> crend() const noexcept
			{
				return rend<I>();
			}

			iterator begin() noexcept
			{
				return {*this,0};
			}
			const_iterator begin() const noexcept
			{
				return {*this,0};
			}
			const_iterator cbegin() const noexcept
			{
				return {*this,0};
			}
			iterator end() noexcept
			{
				return {*this,size()};
			}
			const_iterator end() const noexcept
			{
				return {*this,size()};
			}
			const_iterator cend() const noexcept
			{
				return {*this,size()};
			}

			reverse_iterator rbegin() noexcept
			{
				return {end()};
			}
			const_reverse_iterator rbegin() const noexcept
			{
				return {end()};
			}
			const_reverse_iterator crbegin() const noexcept
			{
				return {end()};
			}
			reverse_iterator rend() noexcept
			{
				return {begin()};
			}
			const_reverse_iterator rend() const noexcept
			{
				return {begin()};
			}
			const_reverse_iterator crend() const noexcept
			{
				return {begin()};
			}
		protected:
			template<size_t I>
			void move_range(void* new_buffer,size_t new_cap)
			{
				using type=get_t<I>;
				constexpr auto offset_unit=size_up_to<I>::value;
				type* old=reinterpret_cast<type*>(data()+capacity()*offset_unit);
				type* nb=reinterpret_cast<type*>(new_buffer+new_cap*offset_unit);
				for(size_t i=0;i<size();++i)
				{
					do_construct<I>(nb+i,std::move(old[i]));
				}
			}

			void move_ranges(void* new_buffer,size_t new_cap,std::index_sequence<>)
			{}
			template<size_t I,size_t... Is>
			void move_ranges(void* new_buffer,size_t new_cap,std::index_sequence<I,Is...>)
			{
				move_range<I>(new_buffer,new_cap);
				move_ranges(new_buffer,new_cap,std::index_sequence<Is...>());
			}

			template<size_t I>
			void copy_range(void const* src,size_t src_offset)
			{
				using type=get_t<I>;
				constexpr size_t offset=size_up_to<I>::value;
				type* odst=data<I>();
				type const* osrc=reinterpret_cast<type const*>(src+offset*src_offset);
				for(size_t i=0;i<_size;++i)
				{
					try
					{
						do_construct<I>(odst+i,type(osrc[i]));
					}
					catch(...)
					{
						destroy_range(0,i);
						throw;
					}
				}
			}
			template<size_t I,size_t... Is>
			void copy_range(void const* src,size_t src_offset,std::index_sequence<I,Is...>)
			{
				copy_range<I>(src,src_offset);
				try
				{
					copy_range(src,src_offset,std::index_sequence<Is...>());
				}
				catch(...)
				{
					destroy_range(data<I>(),0,size());
					throw;
				}
			}
			void copy_range(void const* src,size_t src_offset,std::index_sequence<>)
			{}

			void realloc(size_t new_cap)
			{
				assert(new_cap%alignment==0);
				_DataM temp(new_cap);
				move_ranges(temp,new_cap,std::make_index_sequence<type_count>());
				//@@ NEED CHECK NOEXCEPT MOVE@@
				temp.swap(_data);
			}

		public:
			mvector(mvector const& other):_data(other.capacity()),_size(other._size)
			{
				copy_range(other.data(),other.capacity(),idx_seq<type_count>());
			}
			mvector(mvector&& other):_data(std::move(other._data)),_size(other._size)
			{
				other._size=0;
			}
			mvector():_data(),_size(0)
			{}
		private:
			template<typename Ret,typename MV,size_t... Is>
			friend Ret subscript_impl(size_t s,MV&& ref,std::index_sequence<Is...>)
			{
				return {static_cast<get_t<I>*>(ref.data()+ref.type_offset<I>())[s]...};
			}
		public:
			reference operator[](size_t s)
			{
				return subscript_impl<reference>(s,*this,idx_seq<type_count>{});
			}
			const_reference operator[](size_t s) const
			{
				return subscript_impl<const_reference>(s,*this,idx_seq<type_count>{});
			}
		private:
			static constexpr size_t fix_alignment(size_t s)
			{
				auto const rem=s%alignment;
				return rem?s+(alignment-rem):s;
			}

			template<size_t I,size_t E>
			struct constructor {
				template<typename... Rest>
				static void construct(size_t begin,size_t end,char* data,size_t cap,get_t<I>&& val,Rest&&... rest)
				{
					get_t<I>* o=reinterpret_cast<get_t<I>*>(data+size_up_to<I>::value*cap);
					for(size_t i=begin;i<end;++i)
					{
						do_construct<I>(o+i,std::forward<get_t<I>>(val));
					}
					constructor<I+1,E>::construct(begin,end,data,cap,std::forward<Rest>(rest)...);
				}
				static void construct(size_t begin,size_t end,char* data,size_t cap)
				{
					get_t<I>* o=reinterpret_cast<get_t<I>*>(data+size_up_to<I>::value*cap);
					for(size_t i=begin;i<end;++i)
					{
						do_construct<I>(o+i);
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
			mvector(size_t s,Args const&... args):_data(fix_alignment(s)),_size(s)
			{
				static_assert(sizeof...(Args)<=type_count);
				constructor<0,type_count>::construct(s,data(),capacity(),args);
			}
			void reserve(size_t s)
			{
				if(s>capacity())
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
				if(new_size>capacity())
				{
					realloc(2*capacity()+alignment);
				}
				constructor<0,type_count>::construct(_size,new_size,data(),capacity(),std::forward<Args>(args)...);
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
						do_construct<B>(data+i,std::forward<First>(f));
					}
					inserter<B+1,E>::insert(pos,remaining,count,base,cap,std::forward<Args>(args)...);
				}
				static void insert(size_t pos,size_t remaining,size_t count,char* base,size_t cap)
				{
					size_t offset=size_up_to<B>::value*cap;
					using Type=get_t<B>;
					Type* data=reinterpret_cast<Type*>(base+offset)+pos;
					std::memmove(data+count,data,remaining*sizeof(Type));
					for(size_t i=0;i<count;++i)
					{
						do_construct<B>(data+i);
					}
					inserter<B+1,E>::insert(pos,remaining,count,base,cap);
				}
			};
			template<size_t B>
			struct inserter<B,B> {
				static void insert(size_t,size_t,size_t,char*,size_t)
				{}
			};

		public:
			template<size_t I,typename Iter,typename... Args>
			void insert_index(size_t index,size_t count,Args&&... args)
			{
				size_t remaining=size()-index;
				size_t new_size=_size+count;
				if(new_size>capacity())
				{
					realloc(fix_alignment(new_size));
				}
				_size=new_size;
				inserter<0,type_count>::insert(index,remaining,count,data(),capacity(),std::forward<Args>(args)...);
			}
		private:

			template<size_t I,typename Iter,typename... Args>
			void insert_impl(Iter pos,size_t count,Args&&... args)
			{
				insert_index(pos-begin<I>,count,std::forward<Args>(args)...);
			}
		public:
			template<size_t I,typename... Args>
			void insert_n(subrange_iterator<I> pos,size_t count,Args const&... args)
			{
				insert_impl<I>(pos,count,args...);
			}
			template<size_t I,typename... Args>
			void insert_n(subrange_iterator<I> pos,size_t count,Args const&... args)
			{
				insert_impl<I>(pos,count,args...);
			}
			template<size_t I,typename... Args>
			void insert(subrange_iterator<I> pos,Args&&... args)
			{
				insert_impl<I>(pos,1,std::forward<Args>(args)...);
			}
			template<size_t I,typename... Args>
			void insert(subrange_iterator<I> pos,Args&&... args)
			{
				insert_impl<I>(pos,1,std::forward<Args>(args)...);
			}

			template<typename... Args>
			void insert(iterator pos,Args&&... args)
			{
				insert_index(pos._index,1,std::forward<Args>(args)...);
			}
			template<typename... Args>
			void insert(const_iterator pos,Args&&... args)
			{
				insert_index(pos._index,1,std::forward<Args>(args)...);
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
		public:
			void erase_index(size_t first,size_t last)
			{
				size_t remaining=size()-last;
				size_t count=last-first;
				eraser<0,type_count>::erase(first,remaining,count,data(),capacity());
				_size-=count;
			}
		private:
			template<size_t I,typename Iter>
			void erase_impl(Iter first,Iter last)
			{
				erase_index(first-begin<I>(),last-begin<I>());
			}
		public:
			template<size_t I>
			void erase(subrange_iterator<I> first,subrange_iterator<I> last)
			{
				erase_impl<I>(first,last);
			}
			template<size_t I>
			void erase(const_subrange_iterator<I> first,const_subrange_iterator<I> last)
			{
				erase_impl<I>(first,last);
			}
			template<size_t I>
			void erase(subrange_iterator<I> first)
			{
				erase_impl<I>(first,first+1);
			}
			template<size_t I>
			void erase(const_subrange_iterator<I> first)
			{
				erase_impl<I>(first,first+1);
			}
			void erase(iterator first)
			{
				erase_index(first._index);
			}
			void erase(iterator first,iterator last)
			{
				erase_index(first._index,last._index);
			}
			void erase(const_iterator first)
			{
				erase_index(first._index);
			}
			void erase(const_iterator first,const_iterator last)
			{
				erase_index(first._index,last._index);
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
					resize_grow(s,idx_seq{});
				}
			}
			mvector& operator=(mvector const& other)
			{
				clear();
				reserve(other.size());
				copy_range(other.data(),other.capacity(),idx_seq{});
				return *this;
			}
			~mvector() noexcept
			{
				eraser<0,type_count>::erase(0,0,size(),data(),capacity());
			}
			mvector& operator=(mvector&& other) noexcept
			{
				eraser<0,type_count>::erase(0,0,_size,data(),capacity());
				_data=std::move(other._data);
				_size=other._size;
				other._size=0;
				return *this;
			}
		};
	}

	//Creates a vector that organizes the given types 
	//as an array of type1 next to array of type2 next to array of type3...
	//all of the same size
	template<typename Type1,typename... Types>
	using multi_vector=detail::mvector<std::tuple<Type1,Types...>,std::allocator<std::tuple<std::allocator<Type1,Types...>>;

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
#undef IFCONSTEXPR
#endif