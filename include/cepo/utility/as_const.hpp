//Copyright (c) 2026 CB604BL
#ifndef CEPO_UTILITY_AS_CONST_HPP
#define CEPO_UTILITY_AS_CONST_HPP

#include "cepo/configs/namespace_macro.h"
#include "cepo/macros/nodiscard.hpp"

CEPO_NAMESPACE_START;

template<typename T>
CEPO_NODISCARD
constexpr const T& as_const(T& lvalue) noexcept
{
	return lvalue;
}

template<typename T>
void as_const(const T&&) = delete;

CEPO_NAMESPACE_END;

#endif //CEPO_UTILITY_AS_CONST_HPP
