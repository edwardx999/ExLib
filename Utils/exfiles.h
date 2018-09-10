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
#ifndef EXFILES_H
#define EXFILES_H
#include <vector>
#ifdef _WINDOWS
#include <Windows.h>
#endif
#include <stdio.h>
namespace exlib {
	/*
		Returns a vector containing the filenames of all files in the first level of the given directory.
	*/
	template<typename String>
	std::vector<String> files_in_dir(String const& path);

	/*
		Returns a String with any consecutive slashes replaced by a single slash.
	*/
	template<typename String,typename U>
	String clean_multislashes(U const* input);

	/*
		Returns a String with any consecutive slashes replaced by a single slash.
	*/
	template<typename String>
	String clean_multislashes(String const& input);

	/*
		Returns any consecutive slahes from the input, and returns the new size of the input string.
	*/
	template<typename T>
	size_t remove_multislashes(T* input);

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
			do
			{
				if(!(fdata.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
				{
					files.emplace_back(fdata.cFileName);
				}
			} while(FindNextFileA(hFind,&fdata));
			FindClose(hFind);
		}
		return files;
	}
#endif

	template<typename String,typename U>
	String clean_multislashes(U const* input)
	{
		String out;
		out.reserve(30);
		bool found=false;
		while(*input)
		{
			if(found)
			{
				if(*input!='\\'&&*input!='/')
				{
					found=false;
					out.push_back(*input);
				}
			}
			else
			{
				if(*input=='\\'||*input=='/')
				{
					found=true;
				}
				out.push_back(*input);
			}
			++input;
		}
		return out;
	}

	template<typename String>
	String clean_multislashes(String const& input)
	{
		return clean_multislashes<String>(input.c_str());
	}

	template<typename T>
	size_t remove_multislashes(T* input)
	{
		T* it=input;
		struct keep {
			T* start;
			T* end;
		};
		std::vector<keep> keeps;
		keeps.push_back({it,nullptr});
		bool found=false;
		int i=0;
		while(*it)
		{
			if(keeps.back().end)//still in valid territory
			{
				if(*it!='/'&&*it!='\\')
				{
					keeps.push_back({it,nullptr});
				}
			}
			else
			{
				if(*it=='/'||*it=='\\')
				{
					keeps.back().end=it+1;
				}
			}
			++it;
		}
		keeps.back().end=it;
		it=input;
		for(auto& k:keeps)
		{
			while(k.start!=k.end)
			{
				*it=*k.start;
				++it;
				++k.start;
			}
		}
		*it=0;
		return it-input;
	}

	template<typename Iter>
	Iter find_extension(Iter begin,Iter end)
	{
		--begin;
		auto it=end-1;
		while(1)
		{
			if(it==begin)
			{
				return end;
			}
			if(*it=='.')
			{
				return it+1;
			}
			if(*it=='\\'||*it=='/')
			{
				return end;
			}
			--it;
		}
	}

	template<typename Iter>
	Iter find_filename(Iter begin,Iter end)
	{
		for(;end!=begin;)
		{
			--end;
			if(*end=='/'||*end=='\\')
			{
				return end+1;
			}
		}
		return end;
	}

	template<typename Iter>
	Iter find_path_end(Iter begin,Iter end)
	{
		for(;end!=begin;)
		{
			--end;
			if(*end=='/'||*end=='\\')
			{
				return end;
			}
		}
		return end;
	}
	
}
#endif