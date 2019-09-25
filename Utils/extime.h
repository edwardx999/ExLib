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
#ifndef EXTIME_H
#define EXTIME_H
#include <chrono>
#include <utility>
#include <type_traits>
namespace exlib {

	template<typename Measure,typename Res>
	struct timing {
		Measure time;
		Res result;
		operator std::pair<Measure,Res>() const&
		{
			return std::pair<Measure,Res>(time,static_cast<Res>(result));
		}
		operator std::pair<Measure,Res>() &&
		{
			return std::pair<Measure,Res>(time,std::forward<Res>(result));
		}
	};

	template<typename Measure>
	struct timing<Measure,void> {
		Measure time;
	};

	namespace detail {
		template<typename M,typename Res>
		struct time_helper {
			template<typename Op,typename... Args>
			static timing<M,Res> get(Op&& op,Args&&... args)
			{
				auto b=std::chrono::steady_clock::now();
				Res res=op(std::forward<Args>(args)...);
				auto e=std::chrono::steady_clock::now();
				M p=std::chrono::duration_cast<M>(e-b);
				return {p,std::forward<Res>(res)};
			}
		};
		template<typename M>
		struct time_helper<M,void> {
			template<typename Op,typename... Args>
			static timing<M,void> get(Op&& op,Args&&... args)
			{
				auto b=std::chrono::steady_clock::now();
				op(std::forward<Args>(args)...);
				auto e=std::chrono::steady_clock::now();
				M p=std::chrono::duration_cast<M>(e-b);
				return {p};
			}
		};
	}
	/*
		Returns a pair of the time it takes to perform op on the args, and the result of that op.
		By default the measure is steady_clock::duration
	*/
	template<typename Measure=typename std::chrono::steady_clock::duration,typename Op,typename... Args>
	constexpr auto time_action(Op&& op,Args&&... args) -> timing<Measure,decltype(std::forward<Op>(op)(std::forward<Args>(args)...))>
	{
		return detail::time_helper<Measure,decltype(std::forward<Op>(op)(std::forward<Args>(args)...))>::get(std::forward<Op>(op),std::forward<Args>(args)...);
	}
}
#endif