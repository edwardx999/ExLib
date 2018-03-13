#ifndef EXMATH_H
#define EXMATH_H
#include <type_traits>
namespace exlib {
	template<typename T>
	unsigned int num_digits(T n,T base=10);

	template<typename T>
	unsigned int num_digits(T num,T base)
	{
		static_assert(std::is_integral<typename T>::value,"Requires integral type");
		unsigned int num_digits=1;
		while((num/=base)!=0)
		{
			++num_digits;
		}
		return num_digits;
	}

	/*
		Little-endian arbitrarily sized 2's complement integer.
	*/
	class BigInteger {
		std::vector<int64_t> data;
	public:
		struct div_t;

		BigInteger(int64_t);

		template<typename T>
		friend void operator<<(std::basic_ostream<T>,BigInteger const&);

		size_t size() const;
		size_t capacity() const;
		void reserve(size_t);

		bool operator==(BigInteger const&) const;
		bool operator<(BigInteger const&) const;
		bool operator>(BigInteger const&) const;

		BigInteger operator+(BigInteger const&) const;
		BigInteger& operator+=(BigInteger const&);
		BigInteger operator-(BigInteger const&) const;
		BigInteger& operator-=(BigInteger const&);
		BigInteger operator-() const;
		BigInteger& negate();
		BigInteger operator*(BigInteger const&) const;
		BigInteger& operator*=(BigInteger const&);
		BigInteger operator/(BigInteger const&) const;
		BigInteger& operator/=(BigInteger const&);
		BigInteger operator%(BigInteger const&) const;
		BigInteger& operator%=(BigInteger const&);
		static div_t div(BigInteger const&,BigInteger const&);

		BigInteger operator>>(size_t) const;
		BigInteger& operator>>=(size_t);
		BigInteger operator<<(size_t) const;
		BigInteger& operator<<=(size_t);
		BigInteger operator&(BigInteger const&) const;
		BigInteger& operator&=(BigInteger const&);
		BigInteger operator|(BigInteger const&) const;
		BigInteger& operator|=(BigInteger const&);
		BigInteger operator^(BigInteger const&) const;
		BigInteger& operator^=(BigInteger const&);
		BigInteger operator~() const;
		BigInteger& bitwise_negate();

		int signum() const;

		operator int64_t() const;
		operator double() const;
	};

	struct BigInteger::div_t {
		BigInteger quot;
		BigInteger rem;
	};
}

namespace std {
	template<>
	exlib::BigInteger const& max<exlib::BigInteger>(exlib::BigInteger const&,exlib::BigInteger const&);

	template<>
	struct is_integral<exlib::BigInteger>:public std::true_type {};

	template<>
	struct is_signed<exlib::BigInteger>:public std::true_type {};
}

namespace exlib {
	inline BigInteger::BigInteger(int64_t val):data(5)
	{
		data.back()|=val;
		data.back()&=0x8000'0000'0000'0000;
		data.front()=val&0x7FFF'FFFF'FFFF'FFFF;
	}
	inline size_t BigInteger::size() const
	{
		return data.size();
	}
	inline size_t BigInteger::capacity() const
	{
		return data.capacity();
	}
	inline void BigInteger::reserve(size_t n)
	{
		auto old_back_val=data.back();
		auto& old_back=data.back();
		data.reserve(n);
		old_back&=0x7FFF'FFFF'FFFF'FFFF;
		data.back()|=old_back_val;
		data.back()&=0x8000'0000'0000'0000;
	}
	inline bool BigInteger::operator==(BigInteger const& other) const
	{
		if(signum()!=other.signum())
		{
			return false;
		}
		size_t limit=std::min(other.size(),size());
	}
}
#endif