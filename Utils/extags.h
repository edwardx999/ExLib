#pragma once
#ifndef EXTAGS_H
#define EXTAGS_H
namespace exlib {
	template<typename T>
	struct in_place_type_t {
		explicit in_place_type_t() noexcept = default;
		using type=T;
	};

	struct default_init_t{};
}
#endif