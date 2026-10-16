//Copyright (c) 2026 CB604BL
#ifndef CEPO_TYPE_TRAITS_IS_UNBOUNDED_ARRAY_HPP
#define CEPO_TYPE_TRAITS_IS_UNBOUNDED_ARRAY_HPP

#include "cepo/configs/namespace_macro.h"
#include <type_traits>

CEPO_NAMESPACE_START;

template<typename T>
struct is_unbounded_array : std::false_type{};

template<typename T>
struct is_unbounded_array<T[]> : std::true_type{};

CEPO_NAMESPACE_END;

#endif //CEPO_TYPE_TRAITS_IS_UNBOUNDED_ARRAY_HPP
