//Copyright (c) 2026 CB604BL
#ifndef CEPO_RESULT_EXCEPTION_STATE_HPP
#define CEPO_RESULT_EXCEPTION_STATE_HPP

#include "cepo/configs/namespace_macro.h"

CEPO_NAMESPACE_START;

enum class result_exception_state
{
	value_construction_exception,
	error_construction_exception,
	no_exception_threw
};

CEPO_NAMESPACE_END;

#endif //CEPO_RESULT_EXCEPTION_STATE_HPP
