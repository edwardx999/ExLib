#ifndef EXTIME_H
#define EXTIME_H
#include <chrono>
#include <utility>
namespace exlib {

	template<typename Measure,typename Res>
	struct timing {
		Measure time;
		Res result;
		operator std::pair<Measure,Res>() const
		{
			return {time,result};
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
				return {p,res};
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
		By default the measure is std::chrono::milliseconds.
	*/
	template<typename Measure=std::chrono::milliseconds,typename Op,typename... Args>
	constexpr auto time_action(Op&& op,Args&&... args) -> timing<Measure,decltype(std::forward<Op>(op)(std::forward<Args>(args)...))>
	{
		return detail::time_helper<Measure,decltype(std::forward<Op>(op)(std::forward<Args>(args)...))>::get(std::forward<Op>(op),std::forward<Args>(args)...);
	}
}
#endif