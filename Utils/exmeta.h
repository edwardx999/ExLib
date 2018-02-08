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
#define include_has_method_h(METHOD_NAME)\
namespace exlib {\
	template<typename T>\
	struct has_ ## METHOD_NAME ## _method\
	{\
	private:\
		template<typename U>\
		static auto check(int) -> decltype(std::declval<U>(). ## METHOD_NAME ## ()==1,std::true_type());\
		template<typename>\
		static std::false_type check(...);\
	public:\
		static constexpr bool value=std::is_same<decltype(check<T>(0)),std::true_type>::value;\
	};\
}
#define make_has_method(METHOD_NAME) include_has_method_h(METHOD_NAME)

#endif
