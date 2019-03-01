#pragma once
#ifndef EXBOUNDVIEW_H
#define EXBOUNDVIEW_H
#include <iterator>
#ifdef _MSVC_LANG
#define _EXBOUNDVIEW_HAS_CPP_17 _MSVC_LANG>=201700L
#else
#define _EXBOUNDVIEW_HAS_CPP_17 _cplusplus>=201700L
#endif
namespace exlib {

	namespace detail {

		template<typename Container>
		struct has_random_access:std::is_base_of<
			std::random_access_iterator_tag,
			typename std::iterator_traits<typename Container::iterator>::iterator_category> {};

		template<typename Container>
		struct has_bidir_access:std::is_base_of<
			std::bidirectional_iterator_tag,
			typename std::iterator_traits<typename Container::iterator>::iterator_category> {};

		template<typename Container>
		class bound_span_base {
		protected:
			Container* _base;
		public:
			using iterator=decltype(std::declval<Container&>().begin());
			using const_iterator=typename Container::const_iterator;
			using value_type=typename Container::value_type;
			using size_type=typename Container::size_type;
			using difference_type=typename Container::difference_type;
			using reference=decltype(*std::declval<Container&>().begin());
			using const_reference=typename Container::const_reference;
			using pointer=decltype(std::declval<Container&>().begin().operator->());
			using const_pointer=typename Container::pointer;
			constexpr bound_span_base(Container& base) noexcept:_base(&base)
			{}
			constexpr bound_span_base(bound_span_base const&) noexcept=default;
			constexpr bound_span_base& operator=(bound_span_base const&) noexcept=default;
			constexpr Container& underlying() const noexcept
			{
				return *_base;
			}
		};

		template<typename Container,bool has_bidir_access=has_bidir_access<Container>::value>
		class linked_span;

		template<typename Container>
		class linked_span<Container,false>:public bound_span_base<Container> {
		public:
			constexpr linked_span(Container& base,
				iterator begin,
				iterator end) noexcept:bound_span_base(base),_begin(begin),_end(end)
			{}
			constexpr linked_span(Container& base,
				iterator begin) noexcept:linked_span(base,begin,base.end())
			{}
			constexpr linked_span(Container& base) noexcept:linked_span(base,base.begin(),base.end())
			{}

			constexpr linked_span(linked_span const&) noexcept=default;
			constexpr linked_span& operator=(linked_span const&) noexcept=default;

			constexpr iterator begin() noexcept
			{
				return _begin;
			}
			constexpr iterator end() noexcept
			{
				return _end;
			}
			constexpr const_iterator begin() const noexcept
			{
				return cbegin();
			}
			constexpr const_iterator end() const noexcept
			{
				return cend();
			}
			constexpr const_iterator cbegin() const noexcept
			{
				return const_iterator{_begin};
			}
			constexpr const_iterator cend() const noexcept
			{
				return const_iterator{_end};
			}
			constexpr bool empty() const noexcept
			{
				return _begin==_end;
			}
			constexpr size_t size() const noexcept
			{
				return std::distance(_begin,_end);
			}
			constexpr reference front() noexcept
			{
				return *begin();
			}
			constexpr const_reference front() const noexcept
			{
				return *begin();
			}
		private:
			iterator _begin;
			iterator _end;
		};

		template<typename Container>
		class linked_span<Container,true>:public linked_span<Container,false> {
			using Base=linked_span<Container,false>;
		public:
			using reverse_iterator=decltype(std::declval<Container&>().rbegin());
			using const_reverse_iterator=decltype(std::declval<Container&>().crbegin());
			using Base::Base;
			constexpr linked_span(linked_span const&) noexcept=default;
			constexpr linked_span& operator=(linked_span const&) noexcept=default;

			constexpr reference back() noexcept
			{
				auto be=this->_end;
				--be;
				return *be;
			}
			constexpr const_reference back() const noexcept
			{
				auto be=this->_end;
				--be;
				return *be;
			}
			constexpr reverse_iterator rbegin() noexcept
			{
				return reverse_iterator(this->_end);
			}
			constexpr reverse_iterator rend() noexcept
			{
				return reverse_iterator(this->_begin);
			}
			constexpr const_reverse_iterator rbegin() const noexcept
			{
				return const_reverse_iterator(this->_end);
			}
			constexpr const_reverse_iterator rend() const noexcept
			{
				return const_reverse_iterator(this->_begin);
			}
			constexpr const_reverse_iterator crbegin() const noexcept
			{
				return const_reverse_iterator(this->_end);
			}
			constexpr const_reverse_iterator crend() const noexcept
			{
				return const_reverse_iterator(this->_begin);
			}
		};

		template<typename Container,bool random_access=has_random_access<Container>::value>
		class bound_span;

		template<typename Container>
		class bound_span<Container,false>:public linked_span<Container> {
			using Base=linked_span<Container>;
		public:
			using Base::Base;
			using typename Base::iterator;
			using typename Base::size_type;
			using typename Base::difference_type;
			constexpr bound_span(bound_span const&) noexcept=default;
			constexpr bound_span& operator=(bound_span const&) noexcept=default;
			constexpr operator bound_span<Container const>() const noexcept
			{
				return bound_span<Container const>(this->underlying(),this->begin(),this->end());
			}

			constexpr bound_span subspan(iterator begin_,iterator end_) const noexcept
			{
				return bound_span(this->underlying(),begin_,end_);
			}
			constexpr bound_span subspan() const noexcept
			{
				return *this;
			}
			constexpr bound_span subspan(iterator begin_) const noexcept
			{
				return bound_span(this->underlying(),begin_,this->end());
			}
			constexpr bound_span subspan(difference_type offset) const noexcept
			{
				auto b=this->begin();
				std::advance(b,offset);
				return bound_span(this->underlying(),b,end());
			}
			constexpr bound_span subspan(difference_type offset,size_type count) const noexcept
			{
				auto b=this->begin();
				std::advance(b,offset);
				auto e=b;
				std::advance(e,count);
				return bound_span(this->underlying(),b,e);
			}
		};

		template<typename Container>
		class bound_span<Container,true>:public bound_span_base<Container> {
		public:
			constexpr bound_span(Container& base,size_t start,size_t count) noexcept:bound_span_base(base),_start(start),_finish(start+count)
			{}
			constexpr bound_span(Container& base,size_t start=0) noexcept:bound_span_base(base),_start(start),_finish(base.size()-start)
			{}
			constexpr bound_span(Container& base,iterator begin,iterator end) noexcept:bound_span(base,begin-base.begin(),end-begin)
			{}
			constexpr bound_span(Container& base,iterator begin,size_t count) noexcept:bound_span(base,begin-base.begin(),begin-base.begin()+count)
			{}
			constexpr bound_span(Container& base,iterator begin) noexcept:bound_span(base,begin,base.end()-begin)
			{}

			constexpr bound_span(bound_span const&) noexcept=default;
			constexpr bound_span& operator=(bound_span const&) noexcept=default;

			constexpr iterator begin() noexcept
			{
				return this->underlying().begin()+_start;
			}
			constexpr iterator end() noexcept
			{
				return this->underlying().begin()+_finish;
			}
			constexpr const_iterator begin() const noexcept
			{
				return cbegin();
			}
			constexpr const_iterator end() const noexcept
			{
				return cend();
			}
			constexpr const_iterator cbegin() const noexcept
			{
				return const_iterator{this->underlying().begin()+_start};
			}
			constexpr const_iterator cend() const noexcept
			{
				return const_iterator{this->underlying().begin()+_finish};
			}
			constexpr bool empty() const noexcept
			{
				return _start==_finish;
			}
			constexpr size_t size() const noexcept
			{
				return _finish-_start;
			}
			constexpr reference front() noexcept
			{
				return *begin();
			}
			constexpr const_reference front() const noexcept
			{
				return *begin();
			}
			constexpr reference back() noexcept
			{
				return *end();
			}
			constexpr const_reference back() const noexcept
			{
				return *end();
			}
			constexpr operator bound_span<Container const>() const noexcept
			{
				return bound_span<Container const>(underlying(),cbegin(),cend());
			}
			constexpr bound_span subspan(iterator begin_,iterator end_) const noexcept
			{
				return bound_span(underlying(),begin_,end_);
			}
			constexpr bound_span subspan(iterator begin_) const noexcept
			{
				return subspan(begin_,end());
			}
			constexpr bound_span subspan() const noexcept
			{
				return *this;
			}
			constexpr bound_span subspan(difference_type offset) const noexcept
			{
				return bound_span(underlying(),_start+offset,this->size()-offset);
			}
			constexpr bound_span subspan(difference_type offset,size_t count) const noexcept
			{
				return bound_span(underlying(),_start+offset,count);
			}
			reference operator[](size_t i) noexcept
			{
				return begin()[i];
			}
			const_reference operator[](size_t i) const noexcept
			{
				return begin()[i];
			}
			reference at(size_t i) noexcept
			{
				auto real_pos=i+_start;
				return this->underlying().at(real_pos);
			}
			const_reference at(size_t i) const noexcept
			{
				auto real_pos=i+_start;
				return this->underlying().at(real_pos);
			}
		private:
			size_t _start;
			size_t _finish;
		};

#ifdef _EXBOUNDVIEW_HAS_CPP_17
		template<typename Container,typename... Args>
		bound_span(Container&,Args...)->bound_span<Container>;
#endif
	}

	/*
		A sub view of a container.
	*/
	template<typename Container>
	using bound_view=detail::bound_span<Container>;

	template<typename Container>
	using const_bound_view=bound_view<Container const>;

	template<typename Container,typename... Args>
	bound_view<Container> make_bound_view(Container& container,Args... args)
	{
		return bound_view<Container>(container,args...);
	}
	template<typename Container,typename... Args>
	const_bound_view<Container> make_const_bound_view(Container& container,Args... args)
	{
		return const_bound_view<Container>(container,args...);
	}
}
#endif