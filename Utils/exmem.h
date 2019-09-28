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
#include "exretype.h"
#include "exiterator.h"
#include "exfinally.h"
#if defined(_WIN32)
#include <malloc.h>
#define _EXMEM_FORCE_INLINE __forceinline
#elif defined(__GNUC__)||defined(__clang__)
#include <alloca.h>
#define _EXMEM_FORCE_INLINE __attribute__((always_inline))
#endif
#if (__cplusplus>=201700l)
#define IFCONSTEXPR constexpr
#else
#define IFCONSTEXPR
#endif
namespace exlib {

	template<typename Allocator>
	class extra_allocator_traits:public std::allocator_traits<Allocator> {
		template<typename T>
		static auto destroy_moved_impl(Allocator& a,T* data) -> decltype(a.destroy(data))
		{
			a.destroy_moved(data);
		}
		template<typename T,typename... Extra>
		static void destroy_moved_impl(Allocator& a,T* data,Extra...)
		{
			destroy_moved_impl2(a,data,noop_destructor_after_move<T>{});
		}

		template<typename T>
		static void destroy_moved_impl2(Allocator& a,T* data,std::true_type)
		{}

		template<typename T>
		static void destroy_moved_impl2(Allocator& a,T* data,std::false_type)
		{
			data->~T();
		}
	public:
		template<typename T>
		static void destroy_moved(Allocator& a,T* data) noexcept
		{
			destroy_moved_impl(a,data);
		}
	};

	//A smart ptr that uses the given allocator to allocate and delete
	//memory is uninitilized, and you are responsible for destroying any constructed objects
	template<typename T,typename Allocator>
	class allocator_ptr:private empty_store<Allocator> {
	private:
		using Base=empty_store<Allocator>;
		using Traits=std::allocator_traits<Allocator>;
		using pocca=typename Traits::propagate_on_container_copy_assignment;
		using pocma=typename Traits::propagate_on_container_move_assignment;
		using pocs=typename Traits::propagate_on_container_swap;
	public:
		using pointer=typename Traits::pointer;
		using element_type=typename Traits::value_type;
		using allocator_type=Allocator;
		using reference=typename std::add_lvalue_reference<element_type>::type;
		using size_type=typename Traits::size_type;
	private:
		pointer _base;
		size_type _capacity;
		Allocator select_on_move(Allocator& a,std::false_type)
		{
			return Allocator();
		}
		Allocator&& select_on_move(Allocator& a,std::true_type)
		{
			return std::move(a);
		}
	public:
		allocator_type& get_allocator_ref() noexcept
		{
			return Base::get();
		}
		allocator_type get_allocator() const noexcept
		{
			return Base::get();
		}
		void release()
		{
			_base=nullptr;
			_capacity=0;
		}
		void reset() noexcept
		{
			reset(nullptr,0);
		}
		void reset(pointer p,std::size_t capacity,Allocator a=Allocator()) noexcept
		{
			Traits::deallocate(this->get_allocator_ref(),_base,_capacity);
			_base=p;
			_capacity=capacity;
			if(pocma::value)
			{
				this->get_allocator_ref()=std::move(a);
			}
		}
		allocator_ptr(std::size_t n,Allocator const& alloc=Allocator()):Base(alloc),_base(Traits::allocate(this->get_allocator_ref(),n)),_capacity(n)
		{}
		allocator_ptr(Allocator const& alloc=Allocator()) noexcept:Base(Traits::select_on_container_copy_construction(alloc)),_base(nullptr),_capacity(0)
		{}
		allocator_ptr(allocator_ptr&& o) noexcept:Base(select_on_move(o.get_allocator_ref(),pocma{})),_base(o._base),_capacity(o._capacity)
		{
			o.release();
		}
		size_type capacity() const noexcept
		{
			return _capacity;
		}
		pointer get() const noexcept
		{
			return _base;
		}
		void swap(allocator_ptr& o) noexcept
		{
			using std::swap;
			if(pocs::value&&this->get_allocator_ref()!=o.get_allocator_ref())
			{
				swap(this->get_allocator_ref(),o.get_allocator_ref());
			}
			swap(_base,o._base);
			swap(_capacity,o._capacity);
		}
		friend void swap(allocator_ptr& a,allocator_ptr& b) noexcept
		{
			a.swap(b);
		}
		allocator_ptr& operator=(allocator_ptr&& o) noexcept
		{
			reset(o._base,o._capacity,o.get_allocator_ref());
			o.release();
			return *this;
		}
		reference operator[](size_type s) const noexcept
		{
			return _base[s];
		}
		reference operator*() const noexcept
		{
			return *_base;
		}
		~allocator_ptr() noexcept
		{
			if(this->get())
			{
				Traits::deallocate(this->get_allocator_ref(),_base,_capacity);
			}
		}
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
		uninitialized_reference(T& base) noexcept:_base(std::addressof(base))
		{}
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
		T& emplace(Args&&... args) noexcept(std::is_nothrow_constructible<T,Args&&...>::value)
		{
			new (_base)(std::forward<Args>(args)...);
			return *_base;
		}
		explicit operator T& () const noexcept
		{
			return *_base;
		}
		uninitialized_reference& operator=(uninitialized_reference const&) noexcept=default;
		uninitialized_reference& operator=(uninitialized_reference&&) noexcept=default;
		void swap(uninitialized_reference& other) noexcept
		{
			std::swap(_base,other._base);
		}
		friend void swap(uninitialized_reference& a,uninitialized_reference& b) noexcept
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
		constexpr uninitialized_reference<T> operator*() const noexcept
		{
			return *base();
		}
	private:
		constexpr T* operator->() const noexcept
		{
			return base();
		}
	public:
		constexpr uninitialized_reference<T> operator[](size_t s) const noexcept
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
		constexpr bool operator==(default_init_allocator const&) const noexcept
		{
			return true;
		}
		constexpr bool operator!=(default_init_allocator const&) const noexcept
		{
			return false;
		}
	};

	template<typename T,typename Base=std::allocator<T>>
	class no_init_allocator:public Base {
	public:
		using Base::Base;
		template<typename... Args>
		void construct(T* place,Args&&... args)
		{}
		constexpr bool operator==(no_init_allocator const&) const noexcept
		{
			return true;
		}
		constexpr bool operator!=(no_init_allocator const&) const noexcept
		{
			return false;
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
		struct tuple_size_sum<std::tuple<First,Rest...>>:std::integral_constant<std::size_t,sizeof(First)+tuple_size_sum<std::tuple<Rest...>>::value> {};
#endif
		template<std::size_t N>
		class mvector_to_alloc {
			char data[N];
		};
	}

	template<typename T>
	struct pointerlike_wrapper {
	private:
		T mutable _value;
	public:
		template<typename... Args>
		pointerlike_wrapper(Args&&... v):_value(std::forward<Args>(v)...)
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

	enum memory_constants:std::size_t {
		malloc_alignment=
#if defined(_WIN32)||defined(__GNUC__)
#if defined(_WIN64)||defined(__x86_64__)  
		16
#else
		8
#endif
#else
		alignof(std::max_align_t)
#endif
	};

	namespace exmem_detail {
		struct basic_allocator_types {
			using value_type=void;
			using pointer=void*;
			using const_pointer=void const*;
			using void_pointer=void*;
			using difference_type=std::ptrdiff_t;
			using propagate_on_container_copy_assignment=std::false_type;
			using propagate_on_container_move_assignment=std::false_type;
			using propagate_on_container_swap=std::false_type;
			using is_always_equal=std::true_type;

		};
		template<std::size_t ItemSize,std::size_t Alignment,bool Overaligned=(Alignment>memory_constants::malloc_alignment)>
		class buffer_allocator:public basic_allocator_types {
		public:
			pointer allocate(std::size_t n) const
			{
				return new char[n*ItemSize];
			}
			void deallocate(pointer p,std::size_t) const noexcept
			{
				delete[] p;
			}
		};

		template<std::size_t ItemSize,std::size_t Alignment>
		class buffer_allocator<ItemSize,Alignment,true>:public basic_allocator_types {
		public:
			pointer allocate(std::size_t n) const
			{
				auto const ptr=
#ifdef _WIN32
					_aligned_malloc(n*ItemSize,Alignment);
#else
					aligned_alloc(Alignment,n*ItemSize);
#endif
				if(!ptr) throw std::bad_alloc{};
			}
			void deallocate(pointer p,std::size_t) const noexcept
			{
#ifdef _WIN32
				_aligned_free(p);
#else
				free(ptr);
#endif
			}
		};

		template<std::size_t IS,std::size_t A,bool OA>
		constexpr bool operator==(buffer_allocator<IS,A,OA> a,buffer_allocator<IS,A,OA> b) noexcept
		{
			return true;
		}
		template<std::size_t IS,std::size_t A,bool OA>
		constexpr bool operator!=(buffer_allocator<IS,A,OA> a,buffer_allocator<IS,A,OA> b) noexcept
		{
			return false;
		}
	}

	template<std::size_t ItemSize=1,std::size_t Alignment=alignof(std::max_align_t)>
	using buffer_allocator=exmem_detail::buffer_allocator<ItemSize,Alignment>;

	_EXMEM_FORCE_INLINE inline void* stack_alloc(std::size_t bytes) noexcept
	{
#if defined(_WIN32)
		return _alloca(bytes);
#elif defined(__GNUC__)||defined(__clang__)
		return alloca(bytes);
#endif
	}

	/*
		Add subrange (alias for span) method to iterator each subrange
		Fix constructors to use constructors
	*/
	namespace detail {
		template<typename Types,typename Allocator>
		class mvector;

		/*
			Provide a allocator that allocates n*sum_of_sizes bytes given n as an argument to allocate
		*/
		template<typename... Types,typename Allocator>
		class mvector<std::tuple<Types...>,Allocator> {
			static_assert(exlib::value_conjunction<!std::is_reference<Types>::value...>::value,"No references");
		protected:
			using TypeTuple=std::tuple<Types...>;
		public:
			static constexpr std::size_t type_count=std::tuple_size<TypeTuple>::value;
			using allocator_type=Allocator;
		private:
			using AllocTraits=extra_allocator_traits<allocator_type>;
			template<std::size_t I>
			using get_t=typename std::tuple_element<I,TypeTuple>::type;

			static constexpr auto noexcept_movable=exlib::value_conjunction<std::is_nothrow_move_constructible<Types>::value...>::value;
		public:
			using size_type=typename AllocTraits::size_type;
			using difference_type=typename AllocTraits::difference_type;

			using value_type=std::tuple<Types...>;
			using reference=multi_reference<Types&...>;
			using const_reference=multi_reference<Types const&...>;
			using pointer=pointerlike_wrapper<reference>;
			using const_pointer=pointerlike_wrapper<const_reference>;
		private:
			using _DataM=allocator_ptr<void,Allocator>;
			_DataM _data;
			size_type _size;

			constexpr static auto total_size=exlib::detail::tuple_size_sum<TypeTuple>::value;

			using idx_seq=make_index_sequence<type_count>;

			template<std::size_t I,std::size_t... Is>
			static constexpr size_type get_alignment(index_sequence<I,Is...>)
			{
				return std::max<size_type>(alignof(get_t<I>),get_alignment(index_sequence<Is...>()));
			}

			static constexpr size_type get_alignment(index_sequence<>)
			{
				return 1;
			}
		protected:
			static constexpr size_type alignment=get_alignment(idx_seq{});
		private:
			template<typename Ret,typename MV,std::size_t... Is>
			friend Ret subscript_impl(size_type s,MV& ref,std::index_sequence<Is...>)
			{
				return {ref.data<Is>()[s]...};
			}
		public:
			reference operator[](size_type s)
			{
				return subscript_impl<reference>(s,*this,idx_seq{});
			}
			const_reference operator[](size_type s) const
			{
				return subscript_impl<const_reference>(s,*this,idx_seq{});
			}
			template<std::size_t I>
			using subrange_value_type=get_t<I>;
			template<std::size_t I>
			using subrange_reference=get_t<I> &;
			template<std::size_t I>
			using const_subrange_reference=get_t<I> const&;
			template<std::size_t I>
			using subrange_pointer=get_t<I>*;
			template<std::size_t I>
			using const_subrange_pointer=get_t<I> const*;

			class const_iterator;

			class iterator {
			public:
				friend class const_iterator;
				using iterator_category=std::random_access_iterator_tag;
				using value_type=typename mvector::value_type;
				using difference_type=std::ptrdiff_t;
				using reference=typename mvector::reference;
				using pointer=pointerlike_wrapper<reference>;
				using const_reference=typename mvector::const_reference;
				using const_pointer=pointerlike_wrapper<const_reference>;
				friend class multi_vector;
			private:
				mvector* _parent;
				size_t _index;
			public:
				iterator(mvector& parent,size_t index):_parent(&parent),_index(index)
				{}
				iterator(iterator const&) noexcept=default;
				iterator& operator=(iterator const&) noexcept=default;
#define iterator_op(op) iterator operator op(size_t s) const noexcept {return iterator(*_parent,_index op s);} iterator& operator op##=(size_t s) noexcept { _index op##= s; return *this;} 
				iterator_op(+)
				iterator_op(-)
#undef iterator_op
#define iterator_comp(op) bool operator op(iterator const& o) const {assert(_parent==o._parent);return _index op o._index;}
				EXLIB_FOR_ALL_COMP_OPS(iterator_comp)
#undef iterator_comp
				reference operator[](size_t s) const noexcept
				{
					return (*_parent)[s+_index];
				}
				reference operator*() const noexcept
				{
					return (*_parent)[_index];
				}
				pointer operator->() const noexcept
				{
					return wrap_pointerlike((*_parent)[_index]);
				}
				difference_type operator-(iterator const& other) const noexcept
				{
					return _index-other._index;
				}
#define iterator_ment(op) iterator& operator ##op##() noexcept { op##_index;return *this;} iterator operator op(int) noexcept {iterator copy(*this); op##_index;return copy;} 
				iterator_ment(++)
				iterator_ment(--)
#undef iterator_ment
			};

			class const_iterator {
			public:
				using iterator_category=std::random_access_iterator_tag;
				using value_type=typename mvector::value_type;
				using difference_type=typename mvector::difference_type;
				using const_reference=typename mvector::mvector;
				using const_pointer=pointerlike_wrapper<const_reference>;
				using pointer=const_pointer;
				using reference=const_reference;
				friend class multi_vector;
			private:
				mvector const* _parent;
				size_t _index;
			public:
				const_iterator(mvector const& parent,size_t index) noexcept:_parent(&parent),_index(index)
				{}
				const_iterator(iterator const& o) noexcept:const_iterator(o._parent,o._index)
				{}
				const_iterator(const_iterator const&) noexcept=default;
				const_iterator& operator=(const_iterator const&) noexcept=default;
#define iterator_op(op) const_iterator operator op(size_t s) const {return const_iterator(*_parent,_index op s);} const_iterator& operator op##=(size_t s){return _index op##= s,*this;} 
				iterator_op(+)
				iterator_op(-)
#undef iterator_op
#define iterator_comp(op) bool operator op(const_iterator const& o) const noexcept {assert(_parent==o._parent);return _index op o._index;}
				EXLIB_FOR_ALL_COMP_OPS(iterator_comp)
#undef iterator_comp
				reference operator[](size_t s) const noexcept
				{
					return (*_parent)[s];
				}
				reference operator*() const noexcept
				{
					return (*_parent)[0];
				}
				reference operator->() const noexcept
				{
					return wrap_pointerlike((*_parent)[0]);
				}
				difference_type operator-(const_iterator const& other) const noexcept
				{
					return _index-other._index;
				}
				friend difference_type operator-(iterator const& a,const_iterator const& b) noexcept
				{
					return a._index-b._index;
				}
				friend difference_type operator-(const_iterator const& a,iterator const& b) noexcept
				{
					return a._index-b._index;
				}
#define iterator_ment(op) const_iterator& operator op () noexcept { op##_index;return *this;} const_iterator operator op(int) noexcept {const_iterator copy(*this); op##_index;return copy;} 
				iterator_ment(++)
				iterator_ment(--)
#undef iterator_ment
			};

			using reverse_iterator=std::reverse_iterator<iterator>;
			using const_reverse_iterator=std::reverse_iterator<const_iterator>;

			template<std::size_t I>
			struct const_subrange_iterator:iterator_base<get_t<I> const,const_subrange_iterator<I>> {
				using iterator_base::iterator_base;
			};
			template<std::size_t I>
			struct subrange_iterator:iterator_base<get_t<I>,subrange_iterator<I>> {
				using iterator_base::iterator_base;
			};

			template<std::size_t I>
			struct reverse_subrange_iterator:riterator_base<get_t<I>,reverse_subrange_iterator<I>> {
				using riterator_base::riterator_base;
			};
			template<std::size_t I>
			struct const_reverse_subrange_iterator:riterator_base<get_t<I> const,const_reverse_subrange_iterator<I>> {
				using riterator_base::riterator_base;
			};

		protected:

			template<std::size_t I,typename... Args>
			void do_construct(get_t<I>* location,Args&&... args)
			{
				AllocTraits::construct(_data.get_allocator_ref(),location,std::forward<Args>(args)...);
			}
			template<std::size_t I>
			void do_destroy(get_t<I>* location)
			{
				AllocTraits::destroy(_data.get_allocator_ref(),location);
			}

			template<std::size_t I>
			void do_destroy_moved(get_t<I>* location)
			{
				AllocTraits::destroy_moved(_data.get_allocator_ref(),location);
			}
		private:
			template<std::size_t B,std::size_t E>
			struct size_up_to_h:std::integral_constant<size_t,sizeof(get_t<B>)+size_up_to_h<B+1,E>::value> {};

			template<std::size_t N>
			struct size_up_to_h<N,N>:std::integral_constant<size_t,0> {};

			template<std::size_t I>
			using size_up_to=size_up_to_h<0,I>;
		public:
			size_type capacity() const noexcept
			{
				return _data.capacity();
			}
			static constexpr size_type max_size() noexcept
			{
				return std::numeric_limits<size_type>::max()/TotalSize;
			}
		protected:
			template<std::size_t I>
			size_type type_offset() const
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
			size_type size() const noexcept
			{
				return _size;
			}
			bool empty() const noexcept
			{
				return size==0;
			}

			template<std::size_t I>
			get_t<I> const* data() const noexcept
			{
				return reinterpret_cast<get_t<I> const*>(static_cast<char*>(data())+type_offset<I>());
			}
			template<std::size_t I>
			get_t<I>* data() noexcept
			{
				return reinterpret_cast<get_t<I>*>(static_cast<char*>(data())+type_offset<I>());
			}

			template<std::size_t I>
			subrange_iterator<I> begin() noexcept
			{
				return data<I>();
			}
			template<std::size_t I>
			const_subrange_iterator<I> begin() const noexcept
			{
				return data<I>();
			}
			template<std::size_t I>
			const_subrange_iterator<I> cbegin() const noexcept
			{
				return begin<I>();
			}
			template<std::size_t I>
			subrange_iterator<I> end() noexcept
			{
				return data<I>()+size();
			}
			template<std::size_t I>
			const_subrange_iterator<I> end() const noexcept
			{
				return data<I>()+size();
			}
			template<std::size_t I>
			const_subrange_iterator<I> cend() const noexcept
			{
				return end<I>();
			}

			template<std::size_t I>
			reverse_subrange_iterator<I> rbegin() noexcept
			{
				return end();
			}
			template<std::size_t I>
			const_reverse_subrange_iterator<I> rbegin() const noexcept
			{
				return end();
			}
			template<std::size_t I>
			const_reverse_subrange_iterator<I> crbegin() const noexcept
			{
				return end();
			}
			template<std::size_t I>
			reverse_subrange_iterator<I> rend() noexcept
			{
				return begin();
			}
			template<std::size_t I>
			const_reverse_subrange_iterator<I> rend() const noexcept
			{
				return begin();
			}
			template<std::size_t I>
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

			template<std::size_t I>
			void destroy_range(get_t<I>* data,size_type n)
			{
				for(size_type i=0;i<n;++i)
				{
					do_destroy<I>(data+i);
				}
			}

			template<std::size_t I,std::size_t... Is>
			void destroy_ranges(size_type first,size_type last,index_sequence<I,Is...>)
			{
				destroy_range<I>(data<I>()+first,last-first);
				destroy_ranges(first,last,index_sequence<Is...>{});
			}

			void destroy_ranges(size_type,size_type,index_sequence<>)
			{}

			template<std::size_t I>
			void destroy_moved_range(get_t<I>* data,size_type n)
			{
				for(size_type i=0;i<n;++i)
				{
					do_destroy_moved<I>(data+i);
				}
			}

			template<std::size_t I,std::size_t... Is>
			void destroy_moved_ranges(size_type first,size_type last,index_sequence<I,Is...>)
			{
				destroy_moved_range<I>(data<I>()+first,last-first);
				destroy_moved_ranges(first,last,index_sequence<Is...>{});
			}

			void destroy_moved_ranges(size_type,size_type,index_sequence<>)
			{}

			template<std::size_t I>
			void move_range(void* new_buffer,size_type new_cap)
			{
				using type=get_t<I>;
				constexpr auto offset_unit=size_up_to<I>::value;
				type* old=data<I>();
				type* nb=reinterpret_cast<type*>(reinterpret_cast<char*>(new_buffer)+new_cap*offset_unit);
				for(size_t i=0;i<size();++i)
				{
					do_construct<I>(nb+i,std::move(old[i]));
					do_destroy_moved<I>(old+i);
				}
			}

			void move_ranges(void* new_buffer,size_type new_cap,index_sequence<>)
			{}

			template<std::size_t I,std::size_t... Is>
			void move_ranges(void* new_buffer,size_type new_cap,index_sequence<I,Is...>)
			{
				move_range<I>(new_buffer,new_cap);
				move_ranges(new_buffer,new_cap,std::index_sequence<Is...>());
			}

			template<std::size_t I>
			void copy_range(void const* src,size_type src_offset)
			{
				using type=get_t<I>;
				constexpr size_t offset=size_up_to<I>::value;
				type* odst=data<I>();
				type const* osrc=reinterpret_cast<type const*>(reinterpret_cast<char const*>(src)+offset*src_offset);
				for(size_t i=0;i<_size;++i)
				{
					try
					{
						do_construct<I>(odst+i,osrc[i]);
					}
					catch(...)
					{
						destroy_range<I>(odst,i);
						throw;
					}
				}
			}
			template<std::size_t I,std::size_t... Is>
			void copy_ranges(void const* src,size_type src_offset,index_sequence<I,Is...>)
			{
				copy_range<I>(src,src_offset);
				try
				{
					copy_ranges(src,src_offset,std::index_sequence<Is...>());
				}
				catch(...)
				{
					destroy_range<I>(data<I>(),size());
					throw;
				}
			}
			void copy_ranges(void const* src,size_type src_offset,index_sequence<>)
			{}

			void realloc(size_type new_cap)
			{
				assert(new_cap%alignment==0);
				_DataM temp(new_cap);
				if(noexcept_movable)
				{
					move_ranges(temp.get(),new_cap,idx_seq{});
				}
				else
				{
					copy_ranges(temp.get(),new_cap,idx_seq{});
				}
				temp.swap(_data);
			}

		public:
			mvector(mvector const& other):_data(other.capacity()),_size(other._size)
			{
				copy_ranges(other.data(),other.capacity(),idx_seq{});
			}
			mvector(mvector&& other) noexcept:_data(std::move(other._data)),_size(other._size)
			{
				other._size=0;
			}
			mvector() noexcept:_data(),_size(0)
			{}
		private:
			static size_t fix_alignment(size_t count)
			{
				if (type_count<2)
				{
					return count;
				}
				else
				{
					auto const space=count;
					auto const rem=space%alignment;
					return rem?space+(alignment-rem):space;
				}
			}

			template<std::size_t I,typename... Args>
			void construct_range(size_type first,size_type last,Args&&... args)
			{
				auto const loc=data<I>();
				for(size_type i=first;i<last;++i)
				{
					try
					{
						do_construct<I>(loc+i,std::forward<Args>(args)...);
					}
					catch(...)
					{
						destroy_range<I>(loc+first,last-i);
						throw;
					}
				}
			}

			template<std::size_t I,std::size_t... Is,typename Arg1,typename... Args>
			void construct_ranges(size_type first,size_type last,index_sequence<I,Is...>,Arg1&& arg,Args&&... args)
			{
				construct_range<I>(first,last,std::forward<Arg1>(arg));
				try
				{
					construct_ranges(first,last,index_sequence<Is...>{},std::forward<Args>(args)...);
				}
				catch(...)
				{
					destroy_range<I>(data<I>()+first,last-first);
				}
			}
			template<std::size_t I,std::size_t... Is>
			void construct_ranges(size_type first,size_type last,index_sequence<I,Is...>)
			{
				construct_range<I>(first,last);
				try
				{
					construct_ranges(first,last,index_sequence<Is...>{});
				}
				catch(...)
				{
					destroy_range<I>(data<I>()+first,last-first);
				}
			}

			template<typename... Args>
			void construct_ranges(size_type first,size_type last,index_sequence<>,Args const&... args)
			{}

		public:
			template<typename... Args>
			mvector(size_type s,Args const&... args):_data(fix_alignment(s)),_size(s)
			{
				static_assert(sizeof...(Args)<=type_count,"Too many arguments");
				construct_ranges(0,size(),idx_seq{},args...);
			}
			void reserve(size_type s)
			{
				if(s>capacity())
				{
					realloc(fix_alignment(s));
				}
			}
			// directly change size count, no adjustments made
			void force_set_size(size_type s) noexcept
			{
				_size=s;
			}
			void shrink_to_fit()
			{
				auto const new_cap=fix_alignment(_size);
				if(new_cap<capacity())
				{
					realloc(new_cap);
				}
			}

			template<typename... Args>
			void push_back(Args&&... args)
			{
				static_assert(sizeof...(args)<=type_count,"Too many arguments");
				auto const new_size=_size+1;
				if(new_size>capacity())
				{
					realloc(2*capacity()+alignment);
				}
				push_back_unchecked(std::forward<Args>(args)...);
			}

			template<typename... Args>
			void push_back_unchecked(Args&&... args)
			{
				static_assert(sizeof...(args)<=type_count,"Too many arguments");
				auto const new_size=_size+1;
				construct_ranges(_size,new_size,idx_seq{},std::forward<Args>(args)...);
				_size=new_size;
			}

		private:
			template<std::size_t I,std::size_t... Is>
			void erase_impl(size_type first,size_type last,index_sequence<I,Is...>)
			{
				auto const loc=data<I>()+first;
				size_type const dist=last-first;
				size_type const off_end=size()-last;
				auto const near_end=loc+dist;
				if(dist<=off_end)
				{
					for(size_type i=0;i<dist;++i)
					{
						loc[i]=std::move(near_end[i]);
					}
					destroy_range<I>(near_end,off_end-dist);
				}
				else
				{
					for(size_type i=0;i<off_end;++i)
					{
						loc[i]=std::move(near_end[i]);
					}
					auto const start=loc+off_end;
					destroy_range<I>(start,near_end-start);
				}
				erase_impl(first,last,index_sequence<Is...>{});
			}
			void erase_impl(size_type first,size_type last,index_sequence<>)
			{}
		public:
			size_type erase(size_type index) noexcept
			{
				return erase(index,index+1);
			}

			size_type erase(size_type first,size_t last) noexcept
			{
				erase_impl(first,last,idx_seq{});
				_size-=(last-first);
				return first;
			}

			template<std::size_t I>
			subrange_iterator<I> erase(const_subrange_iterator<I> first,const_subrange_iterator<I> last) noexcept
			{
				auto const dist=erase(dist,last-cbegin<I>());
				return begin<I>()+dist;
			}
			template<std::size_t I>
			subrange_iterator<I> erase(subrange_iterator<I> first,subrange_iterator<I> last) noexcept
			{
				auto const dist=erase(first-begin<I>(),last-begin<I>());
				return begin<I>()+dist;
			}
			iterator erase(const_iterator first,const_iterator last) noexcept
			{
				auto const dist=erase(first-cbegin(),last-cbegin());
				return begin()+dist;
			}
			iterator erase(iterator first,iterator last) noexcept
			{
				auto const dist=erase(first-begin(),last-begin());
				return begin()+dist;
			}

			template<std::size_t I>
			subrange_iterator<I> erase(const_subrange_iterator<I> first) noexcept
			{
				return erase(first,first+1);
			}
			template<std::size_t I>
			subrange_iterator<I> erase(subrange_iterator<I> first) noexcept
			{
				return erase(first,first+1);
			}
			iterator erase(const_iterator first) noexcept
			{
				return erase(first,first+1);
			}
			iterator erase(iterator first) noexcept
			{
				return erase(first,first+1);
			}

			void pop_back() noexcept
			{
				erase(size()-1,size());
			}
			
			template<typename... Args>
			void resize(size_t s,Args const&... args)
			{
				if(s<size())
				{
					erase(begin()+s,end());
				}
				if(s>size())
				{
					if(s>capacity())
					{
						realloc(fix_aligment(2*capacity()+1));
					}
					construct_ranges(size(),s,idx_seq{},args...);
					_size=s;
				}
			}

			mvector& operator=(mvector const& other)
			{
				if(&other==this) return*this;
				destroy_ranges(0,size(),idx_seq{});
				reserve(other.size());
				copy_ranges(other.data(),other.capacity(),idx_seq{});
				_size=other._size;
				return *this;
			}

			~mvector() noexcept
			{
				destroy_ranges(0,size(),idx_seq{});
			}

			mvector& operator=(mvector&& other) noexcept
			{
				if(&other==this) return*this;
				destroy_ranges(0,size(),idx_seq{});
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
	using multi_vector=detail::mvector<
		std::tuple<Type1,Types...>,
		buffer_allocator<detail::tuple_size_sum<std::tuple<Type1,Types...>>::value,alignof(std::tuple<Type1,Types...>)>>;

	namespace stack_array_detail {

		template<typename T>
		class stack_array {
		private:
			std::size_t _size;
			T* _data;
			static std::size_t aligned_alloc_amount(std::size_t byte_count)
			{
				return byte_count+(alignof(T)-malloc_alignment);
			}
			static T* aligned(T* ptr)
			{
				auto const normalized=reinterpret_cast<std::size_t>(ptr);
				auto const rem=normalized%alignof(T);
				if(rem==0)
				{
					return ptr;
				}
				return reinterpret_cast<T*>(normalized+alignof(T)-rem);
			}
			_EXMEM_FORCE_INLINE static T* allocate(std::size_t n) noexcept
			{
				auto const raw_amount=n*sizeof(T);
				auto const to_alloc=(alignof(T)<=malloc_alignment)?raw_amount:aligned_alloc_amount(raw_amount);
				auto const ptr=static_cast<T*>(
#ifdef NDEBUG
					stack_alloc(to_alloc)
#else
					malloc(to_alloc)
#endif
					);
				assert(ptr);
#ifdef NDEBUG
				if(alignof(T)>malloc_alignment)
				{
					return aligned(ptr);
				}
#endif
				return ptr;
			}
		public:
			static void* operator new(std::size_t count)=delete;
			static void* operator new[](std::size_t count)=delete;
			using value_type=T;
			using size_type=std::size_t;
			using difference_type=std::ptrdiff_t;
			using reference=T&;
			using const_reference=T const&;
			using pointer=T*;
			using const_pointer=T const*;
			using iterator=pointer;
			using const_iterator=const_pointer;
			using reverse_iterator=std::reverse_iterator<iterator>;
			using const_reverse_iterator=std::reverse_iterator<const_iterator>;

			stack_array(stack_array const&&)=delete;
			stack_array(stack_array&&)=delete;

			_EXMEM_FORCE_INLINE stack_array(stack_array const& other):stack_array(other.begin(),other.end())
			{}

			template<typename Iter>
			_EXMEM_FORCE_INLINE stack_array(Iter begin,Iter end) 
				noexcept(noexcept(std::is_nothrow_constructible<T,decltype(*end)>::value)):
				_size{static_cast<std::size_t>(std::distance(begin,end))},
				_data{allocate(_size)}
			{
				for(auto write_me=data();begin!=end;++begin,++write_me)
				{
					new (write_me) T(*begin);
				}
			}

			_EXMEM_FORCE_INLINE stack_array(std::initializer_list<T> list) 
				noexcept(std::is_nothrow_copy_constructible<T>::value):
				_size{list.size()},
				_data{allocate(_size)}
			{
				for(std::size_t i=0;i<_size;++i)
				{
					new (data()+i) T(list.begin()[i]);
				}
			}

			_EXMEM_FORCE_INLINE stack_array(std::size_t s,T const& val=T()) 
				noexcept(std::is_nothrow_copy_constructible<T>::value):
				_size{s},
				_data{allocate(_size)}
			{
				for(std::size_t i=0;i<_size;++i)
				{
					new (data()+i) T(val);
				}
			}
			_EXMEM_FORCE_INLINE stack_array(std::size_t s,default_init_t) 
				noexcept(std::is_nothrow_default_constructible<T>::value):
				_size{s},
				_data{allocate(_size)}
			{
				if(!std::is_trivial<T>::value)
				{
					new (data()+i) T;
				}
			}
			reference operator[](std::size_t s) noexcept
			{
				return data()[s];
			}
			const_reference operator[](std::size_t s) const noexcept
			{
				return data()[s];
			}
			reference at(std::size_t s)
			{
				if(s>=_size)
				{
					throw std::out_of_range();
				}
				return (*this)[s];
			}
			const_reference at(std::size_t s) const
			{
				if(s>=_size)
				{
					throw std::out_of_range();
				}
				return (*this)[s];
			}
			reference front() noexcept
			{
				return (*this)[0];
			}
			reference back() noexcept
			{
				return (*this)[_size-1];
			}
			T* data() noexcept
			{
#ifdef NDEBUG
				return _data;
#else
				return aligned(_data);
#endif
			}
			T const* data() const noexcept
			{
#ifdef NDEBUG
				return _data;
#else
				return aligned(_data);
#endif
			}
			iterator begin() noexcept
			{
				return data();
			}
			iterator end() noexcept
			{
				return data()+size();
			}
			const_iterator begin() const noexcept
			{
				return data();
			}
			const_iterator end() const noexcept
			{
				return data()+size();
			}
			const_iterator cbegin() const noexcept
			{
				return begin();
			}
			const_iterator cend() const noexcept
			{
				return end();
			}

			reverse_iterator rbegin() noexcept
			{
				return {end()};
			}
			reverse_iterator rend() noexcept
			{
				return {begin()};
			}
			const_reverse_iterator rbegin() const noexcept
			{
				return {end()};
			}
			const_reverse_iterator rend() const noexcept
			{
				return {begin()};
			}
			const_reverse_iterator crbegin() const noexcept
			{
				return {end()};
			}
			const_reverse_iterator crend() const noexcept
			{
				return {begin()};
			}

			bool empty() const noexcept
			{
				return _size==0;
			}
			std::size_t size() const noexcept
			{
				return _size;
			}
			std::size_t capacity() const noexcept
			{
				return _size;
			}
			std::size_t max_size() const noexcept
			{
				return ~0;
			}
			void fill(T const& val) noexcept(std::is_nothrow_copy_assignable<T>::value)
			{
				for(auto& el:*this)
				{
					el=val;
				}
			}
			stack_array& operator=(stack_array const&)=delete;
			~stack_array() noexcept
			{
				if(!std::is_trivially_destructible<T>::value)
				{
					for(auto& val:*this)
					{
						val.~T();
					}
				}
#ifndef NDEBUG
				free(_data);
#endif
			}
		};
	}

	template<typename T>
	using stack_array=stack_array_detail::stack_array<T>;

}
#undef IFCONSTEXPR
#endif
#endif