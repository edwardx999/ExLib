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
#ifndef EXMETA_H
#define EXMETA_H
#include <type_traits>

#define include_has_method_h(METHOD_NAME,OUTPUT,...)\
namespace exlib {\
	template<typename T,typename Output=OUTPUT>\
	struct has_ ## METHOD_NAME ##\
	{\
	private:\
		struct dummy{};\
		template<typename U>\
		static auto check(char) -> decltype(std::declval<U>(). ## METHOD_NAME ## ( ## __VA_ARGS__ ## ));\
		template<typename>\
		static dummy check(...);\
	public:\
		static constexpr bool value=std::is_same<decltype(check<T>(0)),Output>::value;\
	};\
}
#define include_has_method_proc_h(METHOD_NAME,OUTPUT_PROC)\
namespace exlib {\
	template<typename T,typename... Inputs>\
	struct has_ ## METHOD_NAME ## _ ## OUTPUT_PROC {\
	private:\
		struct dummy {};\
		template<typename U>\
		static auto check(char) -> decltype(std::declval<U>(). ## METHOD_NAME ## (std::declval<Inputs>()...));\
		template<typename>\
		static dummy check(...);\
	public:\
		static constexpr bool const value= ## OUTPUT_PROC ## <decltype<check<T>(0)>>;\
	};\
}
/*
	(METHOD_NAME,OUTPUT TYPE,INPUT TYPES)
	This macro creates a template struct with name has_METHOD_NAME_method in the exlib namespace
	that detects whether the given class has a certain member function with specific input and output types.
	This will be replaced by concepts in C++20.
*/
#define make_has_method_concrete(METHOD_NAME,OUTPUT,...) include_has_method_h(METHOD_NAME,OUTPUT,__VA_ARGS__)
#define make_has_method_proc(METHOD_NAME,OUTPUT_PROC) include_has_method_proc_h(METHOD_NAME,OUTPUT_PROC)

//turns a function of one argument into repeated application of the function over multiple paramters
#define multiapp(function_name) \
template<typename T,typename... U> \
void function_name(T const& in,U const&... args) \
{ \
	function_name(in); \
	function_name(args...); \
}



namespace exlib {

	namespace {
		template<typename T>
		struct is_random_access_h {
		private:
			template<typename U>
			constexpr static auto val(int) -> decltype(*std::declval<U>(),std::declval<U>()<std::declval<U>(),std::declval<U>()[size_t()],bool())
			{
				return true;
			}
			template<typename>
			constexpr static bool val(...)
			{
				return false;
			}
		public:
			constexpr static bool value=val<T>(0);
		};

		template<typename T>
		struct is_forward_access_h {
		private:
			template<typename U>
			constexpr static auto val(int) -> decltype(*std::declval<U>(),std::declval<U>()++,bool())
			{
				return true;
			}
			template<typename>
			constexpr static bool val(...)
			{
				return false;
			}
		public:
			constexpr static bool value=val<T>(0);
		};

		template<typename T>
		struct is_bi_access_h {
		private:
			template<typename U>
			constexpr static auto val(int) -> decltype(*std::declval<U>(),std::declval<U>()++,std::declval<U>()--,bool())
			{
				return true;
			}
			template<typename>
			constexpr static bool val(...)
			{
				return false;
			}
		public:
			constexpr static bool value=val<T>(0);
		};
	}

	template<typename T>
	using is_rand_iter=std::integral_constant<bool,is_random_access_h<T>::value>;

	template<typename T>
	using is_forward_iter=std::integral_constant<bool,is_forward_access_h<T>::value>;

	template<typename T>
	using is_bidir_iter=std::integral_constant<bool,is_bi_access_h<T>::value>;

#if __cplusplus>201700L
	template<typename T>
	inline constexpr bool is_rand_iter_v=is_rand_iter<T>::value;

	template<typename T>
	inline constexpr bool is_forward_iter_v=is_forward_iter<T>::value;

	template<typename T>
	inline constexpr bool is_bidir_iter_v=is_bidir_iter<T>::value;
#endif
}
#endif
