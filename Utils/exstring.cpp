/*
Copyright(C) 2017 Edward Xie

This program is free software:you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation,either version 3 of the License,or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.If not,see <https://www.gnu.org/licenses/>.
*/
#include "stdafx.h"
#include "exstring.h"
namespace exstring {
	std::string letter_numbering(size_t num) {
		std::array<char,14> buffer;
		if(num<=25)
			return std::string(1,static_cast<char>('a'+num%26));
		unsigned int index=buffer.size()-1;
		buffer[index]='\0';
		while(num>25)
		{
			buffer[--index]='a'+num%26;
			num/=26;
		}
		buffer[--index]='a'+num%26-1;
		return std::string(buffer.data()+index);
	}
}