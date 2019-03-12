#pragma once
#ifndef EXFINALLY_H
#define EXFINALLY_H
#ifdef _MSVC_LANG
#define EXFINALLY_HAS_CPP17 _MSVC_LANG>=201700l
#else
#define EXFINALLY_HAS_CPP17 __cplusplus>=201700l
#endif
#if EXFINALLY_HAS_CPP17
#define EXFINALLY_NODISCARD [[nodiscard]]
#else
#define EXFINALLY_NODISCARD
#endif
#include <type_traits>
#include <cstring>
namespace exlib {

	template<typename Finally>
	struct EXFINALLY_NODISCARD finally:Finally {
	public:
		template<typename F>
		finally(F&& f):Finally(std::forward<F>(f)){
		}

		~finally()
		{
			Finally::operator()();
		}
	};

#if EXFINALLY_HAS_CPP17
	template<typename F>
	finally(F&& f)->finally<std::decay_t<F>>;
#endif
}
#endif