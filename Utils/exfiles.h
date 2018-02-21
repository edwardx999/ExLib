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
#ifndef EXFILES_H
#define EXFILES_H
#include <vector>
#ifdef _WINDOWS
#include <Windows.h>
#endif
#include <stdio.h>
namespace exlib {
	template<typename String>
	std::vector<String> files_in_dir(String const& path);

#ifdef _WINDOWS
	template<typename String>
	std::vector<String> files_in_dir(String const& path)
	{
		String search=path+"*.*";
		HANDLE hFind;
		WIN32_FIND_DATAA fdata;
		hFind=FindFirstFileA(search.c_str(),&fdata);
		std::vector<String> files;
		if(hFind!=INVALID_HANDLE_VALUE)
		{
			FindNextFileA(hFind,&fdata);
			FindNextFileA(hFind,&fdata);//gets rid of . and ..
			do
			{
				files.emplace_back(fdata.cFileName);
			} while(FindNextFileA(hFind,&fdata));
		}
		FindClose(hFind);
		return files;
	}
#endif
}
#endif