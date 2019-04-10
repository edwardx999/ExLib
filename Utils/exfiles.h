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
#include <regex>
#include <string>
#ifdef _MSVC_LANG
#define _EXFILES_HAS_CPP_20 (_MSVC_LANG>=202000L)
#define _EXFILES_HAS_CPP_17 (_MSVC_LANG>=201700L)
#define _EXFILES_HAS_CPP_14 (_MSVC_LANG>=201400L)
#else
#define _EXFILES_HAS_CPP_20 (__cplusplus>=202000L)
#define _EXFILES_HAS_CPP_17 (__cplusplus>=201700L)
#define _EXFILES_HAS_CPP_14 (__cplusplus>=201400L)
#endif
namespace exlib {

#ifdef _WINDOWS
	/*
		Returns a vector containing the filenames of all files in the first level of the given directory.
	*/
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
	/*
		Returns a String with any consecutive slashes replaced by a single slash.
	*/
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

	/*
		Returns a String with any consecutive slashes replaced by a single slash.
	*/
	template<typename String>
	String clean_multislashes(String const& input)
	{
		return clean_multislashes<String>(input.c_str());
	}

	template<typename T>
	constexpr bool is_slash(T c)
	{
		return c=='\\'||c=='/';
	}

	namespace detail {
		template<typename Iter>
		constexpr std::pair<Iter,Iter> find_multi_slash_block(Iter input) noexcept
		{
			auto& slash_end=input;
			while(true)
			{
				for(;; ++slash_end)
				{
					auto const c=*slash_end;
					if(c=='\0')
					{
						return {slash_end,slash_end};
					}
					if(is_slash(c))
					{
						break;
					}
				}
				auto slash_start=++slash_end;
				if(!is_slash(*slash_start))
				{
					++input;
					continue;
				}
				++slash_end;
				for(;; ++slash_end)
				{
					auto const c=*slash_end;
					if(!is_slash(c))
					{
						return {slash_start,slash_end};
					}
				}
			}
		}
		template<typename Iter>
		constexpr Iter slash_copy(Iter write_head,Iter start,Iter end) noexcept
		{
			for(;;)
			{
				*write_head=*start;
				++write_head;
				++start;
				if(start==end)
				{
					return write_head;
				}
			}
		}
		template<typename Iter>
		constexpr std::size_t remove_multislashes(Iter input) noexcept
		{
			auto sb=find_multi_slash_block(input);
			if(sb.second==sb.first)
			{
				return sb.second-input;
			}
			auto write_head=sb.first;
			while(true)
			{
				auto next=find_multi_slash_block(sb.second);
				if(next.first==next.second)
				{
					if(sb.second!=next.first)
					{
						write_head=slash_copy(write_head,sb.second,next.first);
					}
					*write_head='\0';
					return write_head-input;
				}
				write_head=slash_copy(write_head,sb.second,next.first);
				sb=next;
			}
		}
	}

	/*
		Returns any consecutive slahes from the null-terminated input, and returns the new size of the input string.
	*/
	template<typename CharType>
	constexpr std::size_t remove_multislashes(CharType* input) noexcept
	{
		return detail::remove_multislashes(input);
	}

	/*
		Finds the file extension (beyond the dot)
	*/
	template<typename Iter>
	Iter find_extension(Iter begin,Iter end) noexcept
	{
		if(begin==end)
		{
			return end;
		}
		auto const real_end=end--;
		while(1)
		{
			if(end==begin)
			{
				return real_end;
			}
			if(*end=='.')
			{
				++end;
				return end;
			}
			if(*end=='\\'||*end=='/')
			{
				return real_end;
			}
			--end;
		}
	}

	/*
		Finds the file name (beyond last slash or begin)
	*/
	template<typename Iter>
	Iter find_filename(Iter begin,Iter end) noexcept
	{
		for(; end!=begin;)
		{
			--end;
			if(*end=='/'||*end=='\\')
			{
				return end+1;
			}
		}
		return end;
	}

	namespace detail {
		template<typename Iter,typename Pred>
		Iter find_beyond_last(Iter begin,Pred p) noexcept
		{
			auto last=0;
			for(;;++begin)
			{
				auto const c=*begin;
				if(c=='\0')
				{
					if(last==0)
					{
						return begin;
					}
					return last;
				}
				if(p(c))
				{
					last=++begin;
					continue;
				}
			}
		}
	}
	/*
		Finds last slash or begin (might collide)
	*/
	template<typename Iter>
	Iter find_path_end(Iter begin,Iter end) noexcept
	{
		for(; end!=begin;)
		{
			--end;
			if(*end=='/'||*end=='\\')
			{
				return end;
			}
		}
		return end;
	}

	template<typename Iter>
	Iter find_extension(Iter begin) noexcept
	{
		return detail::find_beyond_last(begin,[](auto c)
			{
				return c=='.';
			});
	}

	template<typename Iter>
	Iter find_filename(Iter begin) noexcept
	{
		return detail::find_beyond_last(begin,[](auto c)
			{
				return is_slash(c);
			});
	}


	template<typename Iter>
	Iter find_path_end(Iter begin) noexcept
	{
		auto last=begin;
		for(;; ++begin)
		{
			auto const c=*begin;
			if(c=='\0')
			{
				return last;
			}
			if(is_slash(c))
			{
				last=begin;
			}
		}
	}

#if _EXFILES_HAS_CPP_17
	namespace get_string_detail {
		template<typename String>
		struct get_string_help_base {
			template<typename Path>
			static auto get(Path const& path) noexcept->decltype(std::enable_if_t<std::is_same_v<typename Path::string_type,String>>(),std::declval<String const&>())
			{
				return path.native();
			}
		};

		template<typename String>
		struct get_string_help;

		template<>
		struct get_string_help<std::string>:get_string_help_base<std::string> {
			using get_string_help_base<std::string>::get;
			template<typename Path,typename... Extra>
			static auto get(Path const& path,Extra...)
			{
				return path.string();
			}
		};

		template<>
		struct get_string_help<std::wstring>:get_string_help_base<std::wstring> {
			using get_string_help_base<std::wstring>::get;
			template<typename Path,typename... Extra>
			static auto get(Path const& path,Extra...)
			{
				return path.wstring();
			}
		};

		template<>
		struct get_string_help<std::u16string>:get_string_help_base<std::u16string> {
			using get_string_help_base<std::u16string>::get;
			template<typename Path,typename... Extra>
			static auto get(Path const& path,Extra...)
			{
				return path.u16string();
			}
		};

		template<>
		struct get_string_help<std::u32string>:get_string_help_base<std::u32string> {
			using get_string_help_base<std::u32string>::get;
			template<typename Path,typename... Extra>
			static auto get(Path const& path,Extra...)
			{
				return path.u32string();
			}
		};

#if _EXFILES_HAS_CPP_20
		template<>
		struct get_string_help<std::u8string>:get_string_help_base<std::u8string> {
			using get_string_help_base<std::u8string>::get;
			template<typename Path,typename... Extra>
			static auto get(Path const& path,Extra...)
			{
				return path.u8string();
			}
		};
#endif

		template<typename String>
		struct get_gen_help;

		template<>
		struct get_gen_help<std::string> {
			template<typename Path>
			static auto get(Path const& path)
			{
				return path.string();
			}
		};
		template<>
		struct get_gen_help<std::wstring> {
			template<typename Path>
			static auto get(Path const& path)
			{
				return path.wstring();
			}
		};
		template<>
		struct get_gen_help<std::u16string> {
			template<typename Path>
			static auto get(Path const& path)
			{
				return path.u16string();
			}
		};
		template<>
		struct get_gen_help<std::u32string> {
			template<typename Path>
			static auto get(Path const& path)
			{
				return path.u32string();
			}
		};
#if _EXFILES_HAS_CPP_20
		template<>
		struct get_gen_help<std::u8string> {
			template<typename Path>
			static auto get(Path const& path)
			{
				return path.u8string();
			}
		};
#endif
	}
	template<typename StringType,typename Path>
	decltype(auto) get_path_string(Path const& path) noexcept(std::is_same_v<typename Path::string_type,StringType>)
	{
		return get_string_detail::get_string_help<StringType>::get(path);
	}

	template<typename StringType,typename Path>
	StringType get_generic_path_string(Path const& path)
	{
		return get_string_detail::get_gen_help<StringType>::get(path);
	}
#endif
}
#endif