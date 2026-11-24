//Copyright (c) 2026 CB604BL
//CEPO - C++ Eleven Patches Open-source
#include "cepo/result/bad_result_unwrap.hpp"
#include "cepo/configs/namespace_macro.hpp"

CEPO_NAMESPACE_START;

bad_result_unwrap::bad_result_unwrap(bad_result_unwrap_reason reason) noexcept:
	r(reason)
{}

const char* bad_result_unwrap::what() const noexcept
{
	switch (r)
	{
	case bad_result_unwrap_reason::unwrap_error_in_ok_state:
		return "cepo::bad_result_unwrap: unwrap_error() called on ok result";

	case bad_result_unwrap_reason::unwrap_value_in_error_state:
		return "cepo::bad_result_unwrap: unwrap_value() called on error result";

	case bad_result_unwrap_reason::unwrap_in_exceptional_state:
		return "cepo::bad_result_unwrap: unwrap_*() called on exceptional result";
	}

	return "cepo::bad_result_unwrap: undefined bad_result_unwrap_reason value";
}

bad_result_unwrap_reason bad_result_unwrap::reason() const noexcept
{
	return r;
}

CEPO_NAMESPACE_END;
