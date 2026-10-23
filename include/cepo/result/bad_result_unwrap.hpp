//Copyright (c) 2026 CB604BL
#ifndef CEPO_RESULT_BAD_RESULT_UNWRAP_HPP
#define CEPO_RESULT_BAD_RESULT_UNWRAP_HPP

#include "cepo/configs/namespace_macro.h"
#include <exception>

CEPO_NAMESPACE_START;

class bad_result_unwrap : public std::exception
{
public:
	enum class reason_type
	{
		is_ok,
		is_error,
		is_empty
	};
	
private:
	reason_type r;

public:
	bad_result_unwrap(reason_type reason) noexcept;
	bad_result_unwrap(const bad_result_unwrap&) noexcept = default;
	bad_result_unwrap& operator=(const bad_result_unwrap&) noexcept = default;
	const char* what() const noexcept override;
	reason_type reason() const noexcept;
	~bad_result_unwrap() noexcept = default;

	bad_result_unwrap() = delete;
};

CEPO_NAMESPACE_END;

#endif //CEPO_RESULT_BAD_RESULT_UNWRAP_HPP
