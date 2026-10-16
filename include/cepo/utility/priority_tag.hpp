//Copyright (c) 2026 CB604BL
#ifndef CEPO_UTILITY_PRIORITY_TAG_HPP
#define CEPO_UTILITY_PRIORITY_TAG_HPP

#include "cepo/configs/namespace_macro.h"
#include <cstddef>

CEPO_NAMESPACE_START;

template<std::size_t priority>
struct priority_tag : priority_tag<priority - 1> {};

template<>
struct priority_tag<0> {};

CEPO_NAMESPACE_END;

#endif //CEPO_UTILITY_PRIORITY_TAG_HPP
