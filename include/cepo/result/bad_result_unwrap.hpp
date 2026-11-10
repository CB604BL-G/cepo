//Copyright (c) 2026 CB604BL
//CEPO - C++ Eleven Patches Open-source
#ifndef CEPO_RESULT_BAD_RESULT_UNWRAP_HPP
#define CEPO_RESULT_BAD_RESULT_UNWRAP_HPP

#include "cepo/configs/namespace_macro.h"
#include <exception>

CEPO_NAMESPACE_START;

enum class bad_result_unwrap_reason_type : unsigned char
{
	unwrap_when_empty,
	unwrap_value_when_error,
	unwrap_error_when_ok
};

class bad_result_unwrap : public std::exception
{	
private:
	bad_result_unwrap_reason_type r;

public:
	bad_result_unwrap(bad_result_unwrap_reason_type reason) noexcept;
	bad_result_unwrap(const bad_result_unwrap&) noexcept = default;
	bad_result_unwrap& operator=(const bad_result_unwrap&) noexcept = default;
	const char* what() const noexcept override;
	bad_result_unwrap_reason_type reason() const noexcept;
	~bad_result_unwrap() noexcept = default;

	bad_result_unwrap() = delete;
};

CEPO_NAMESPACE_END;

#endif //CEPO_RESULT_BAD_RESULT_UNWRAP_HPP
