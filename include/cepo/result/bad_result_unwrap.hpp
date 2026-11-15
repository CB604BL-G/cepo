//Copyright (c) 2026 CB604BL
//CEPO - C++ Eleven Patches Open-source
#ifndef CEPO_RESULT_BAD_RESULT_UNWRAP_HPP
#define CEPO_RESULT_BAD_RESULT_UNWRAP_HPP

#include "cepo/configs/namespace_macro.h"
#include <exception>

CEPO_NAMESPACE_START;

enum class bad_result_unwrap_reason : unsigned char
{
	unwrap_in_exceptional_state,
	unwrap_value_in_error_state,
	unwrap_error_in_ok_state
};

class bad_result_unwrap : public std::exception
{
private:
	bad_result_unwrap_reason r;

public:
	explicit bad_result_unwrap(bad_result_unwrap_reason reason) noexcept;
	bad_result_unwrap(const bad_result_unwrap&) noexcept = default;
	bad_result_unwrap(bad_result_unwrap&&) noexcept = default;
	bad_result_unwrap& operator=(const bad_result_unwrap&) noexcept = default;
	bad_result_unwrap& operator=(bad_result_unwrap&&) noexcept = default;
	~bad_result_unwrap() noexcept override = default;

	const char* what() const noexcept override;
	bad_result_unwrap_reason reason() const noexcept;

	bad_result_unwrap() = delete;
};

CEPO_NAMESPACE_END;

#endif //CEPO_RESULT_BAD_RESULT_UNWRAP_HPP
