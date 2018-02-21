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

#endif
