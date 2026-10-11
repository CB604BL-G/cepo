//Copyright (c) 2026 CB604BL
#ifndef CEPO_TYPE_TRAITS_IS_BOUNDED_ARRAY_HPP
#define CEPO_TYPE_TRAITS_IS_BOUNDED_ARRAY_HPP

#include "configs/namespace_macro.h"
#include <cstddef>
#include <type_traits>

CEPO_NAMESPACE_START;

template<typename T>
struct is_bounded_array : std::false_type{};

template<typename T, std::size_t N>
struct is_bounded_array<T[N]> : std::true_type{};

template<typename T>
struct is_unbounded_array : std::false_type{};

template<typename T>
struct is_unbounded_array<T[]> : std::true_type{};

CEPO_NAMESPACE_END;

#endif //CEPO_TYPE_TRAITS_IS_BOUNDED_ARRAY_HPP
