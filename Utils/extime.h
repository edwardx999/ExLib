#ifndef EXTIME_H
#define EXTIME_H
#include <chrono>
#include <utility>
namespace exlib {

	/*
		Returns a pair of the time it takes to perform op on the args, and the result of that op.
		By default the measure is std::chrono::nanoseconds.
	*/
	template<typename Measure=std::chrono::milliseconds,typename Op,typename... Args>
	constexpr auto time_action(Op op,Args&&... args) -> std::pair<Measure,decltype(op(std::forward<Args>(args)...))>
	{
		using RetType=decltype(op(std::forward<Args>(args)...));
		auto b=std::chrono::high_resolution_clock::now();
		RetType res=op(std::forward<Args>(args)...);
		auto e=std::chrono::high_resolution_clock::now();
		Measure p=b-e;
		return std::pair<Measure,RetType>(p,res);
	}
}
#endif