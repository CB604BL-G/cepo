//Copyright (c) 2026 CB604BL
#ifndef CEPO_CALLABLE_ARG_TYPE_AT_HPP
#define CEPO_CALLABLE_ARG_TYPE_AT_HPP

#include "cepo/callable/traits.hpp"
#include "cepo/configs/namespace_macro.h"
#include <cstddef>

CEPO_NAMESPACE_START;

template<typename Callable, std::size_t index>
using arg_type_at = typename callable_traits<Callable>::template arg_type_at<index>;

CEPO_NAMESPACE_END;

#endif //CEPO_CALLABLE_ARG_TYPE_AT_HPP
