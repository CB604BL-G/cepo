//Copyright (c) 2026 CB604BL
#ifndef CEPO_RESULT_EXCEPTION_CHOOSER_HPP
#define CEPO_RESULT_EXCEPTION_CHOOSER_HPP

#include "cepo/configs/namespace_macro.h"
#include "cepo/result/exception_state.hpp"
#include "cepo/result/slot_tags.hpp"

CEPO_NAMESPACE_START;

namespace impl
{
	template<typename Tag>
	struct result_exception_state_chooser{};

	template<>
	struct result_exception_state_chooser<result_slot_value_tag>
	{
		static constexpr result_exception_state value() noexcept
		{
			return result_exception_state::value_construction_exception;
		}
	};

	template<>
	struct result_exception_state_chooser<result_slot_error_tag>
	{
		static constexpr result_exception_state value() noexcept
		{
			return result_exception_state::error_construction_exception;
		}
	};
}

CEPO_NAMESPACE_END;

#endif //CEPO_RESULT_EXCEPTION_CHOOSER_HPP
