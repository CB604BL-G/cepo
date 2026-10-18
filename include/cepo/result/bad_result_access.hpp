//Copyright (c) 2026 CB604BL
#ifndef CEPO_RESULT_BAD_RESULT_ACCESS_HPP
#define CEPO_RESULT_BAD_RESULT_ACCESS_HPP

#include "cepo/configs/namespace_macro.h"
#include <stdexcept>

CEPO_NAMESPACE_START;

class bad_result_access : public std::logic_error
{
};

CEPO_NAMESPACE_END;

#endif //CEPO_RESULT_BAD_RESULT_ACCESS_HPP
