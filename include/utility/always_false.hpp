//Copyright (c) 2026 CB604BL
#ifndef CEPO_UTILITY_ALWAYS_FALSE_HPP
#define CEPO_UTILITY_ALWAYS_FALSE_HPP

#include "configs/namespace_macro.h"

CEPO_NAMESPACE_START;

template<typename... Ts>
struct always_false
{
	static constexpr bool value = false;
};

template<typename... Ts>
constexpr bool always_false<Ts...>::value;

CEPO_NAMESPACE_END;

#endif //CEPO_UTILITY_ALWAYS_FALSE_HPP
