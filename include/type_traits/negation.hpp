//Copyright (c) 2026 CB604BL
#ifndef CEPO_TYPE_TRAITS_NEGATION_HPP
#define CEPO_TYPE_TRAITS_NEGATION_HPP

#include "configs/namespace_macro.h"
#include <type_traits>

CEPO_NAMESPACE_START;

template<typename B>
struct negation
	: std::integral_constant<bool, not bool(B::value)>
{};

CEPO_NAMESPACE_END;

#endif //CEPO_TYPE_TRAITS_NEGATION_HPP
