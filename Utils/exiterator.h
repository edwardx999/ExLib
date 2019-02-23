#ifndef EXITERATOR_H
#define EXITERATOR_H

namespace exlib {
		/*
		A curiously recurring template pattern base for iterators.
		T is value_type of iterator.
	*/
	template<typename T,typename Derived>
	struct iterator_base {
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
		iterator(const_iterator<T> ci):iterator_base(ci.base())
		{}
	};

	template<typename T>
	struct const_reverse_iterator:iterator_base<T const,const_reverse_iterator<T>> {
		using riterator_base::riterator_base;
	};
	template<typename T>
	struct reverse_iterator:riterator_base<T,reverse_iterator<T>> {
		using riterator_base::riterator_base;
		reverse_iterator(const_reverse_iterator<T> cri):riterator_base(cri.base())
		{}
	};
}

#endif