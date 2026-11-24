//Copyright (c) 2026 CB604BL
//CEPO - C++ Eleven Patches Open-source
#ifndef CEPO_TYPE_TRAITS_BOOL_CONSTANT_HPP
#define CEPO_TYPE_TRAITS_BOOL_CONSTANT_HPP

#include "cepo/configs/namespace_macro.hpp"
#include <type_traits>

CEPO_NAMESPACE_START;

template<bool value>
using bool_constant = std::integral_constant<bool, value>;

CEPO_NAMESPACE_END;

#endif //CEPO_TYPE_TRAITS_BOOL_CONSTANT_HPP
