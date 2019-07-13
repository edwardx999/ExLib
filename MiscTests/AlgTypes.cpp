#include "../Utils/exalg.h"

int main()
{
	constexpr auto wasdd = exlib::conjunction<exlib::is_sized_array<int[5]>, exlib::is_sized_array<int[6]>>::value;
	using A=typename exlib::detail::concat_type<long, int[5],int[7]>::type;
	constexpr auto array=exlib::make_array(1, 2, 3, 4, 5);
	using T=typename exlib::detail::str_concat_type<void, char[6],char[4]>::type;
	constexpr auto catted = exlib::str_concat<char>("hello","bye","D",L"");
}