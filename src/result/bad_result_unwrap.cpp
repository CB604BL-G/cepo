#include "cepo/result/bad_result_unwrap.hpp"
#include "cepo/configs/namespace_macro.h"

CEPO_NAMESPACE_START;

bad_result_unwrap::bad_result_unwrap(bad_result_unwrap::reason_type reason) noexcept:
	r(reason)
{}

const char* bad_result_unwrap::what() const noexcept
{
	switch (r)
	{
	case reason_type::is_ok:
		return "cepo::bad_result_unwrap: try to unwrap an error but result is ok";

	case reason_type::is_error:
		return "cepo::bad_result_unwrap: try to unwrap a value but result is error";

	case reason_type::is_empty:
		return "cepo::bad_result_unwrap: try to unwrap something but result is empty";
	}

	return "cepo::bad_result_unwrap: an undefined enumeration value was passed into this exception object";
}

bad_result_unwrap::reason_type bad_result_unwrap::reason() const noexcept
{
	return r;
}

CEPO_NAMESPACE_END;
