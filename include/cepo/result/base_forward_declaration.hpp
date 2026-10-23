//Copyright (c) 2026 CB604BL
#ifndef CEPO_RESULT_BASE_FORWARD_DECLARATION_HPP
#define CEPO_RESULT_BASE_FORWARD_DECLARATION_HPP

#include "cepo/configs/namespace_macro.h"

CEPO_NAMESPACE_START;

namespace impl
{
	template<typename T, typename E>
	class result_base;

	template<typename Void, typename E>
	class result_base_void;
}

CEPO_NAMESPACE_END;

#endif //CEPO_RESULT_BASE_FORWARD_DECLARATION_HPP
