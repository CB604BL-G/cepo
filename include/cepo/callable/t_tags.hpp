//Copyright (c) 2026 CB604BL
#ifndef CEPO_CALLABLE_T_TAGS_HPP
#define CEPO_CALLABLE_T_TAGS_HPP

#include "cepo/configs/namespace_macro.h"

CEPO_NAMESPACE_START;

namespace impl
{
	struct is_not_class_type{};
	struct is_class_type{};
	struct is_not_with_a_unique_operator{};
	struct is_with_a_unique_operator{};
}

CEPO_NAMESPACE_END;

#endif //CEPO_CALLABLE_T_TAGS_HPP
