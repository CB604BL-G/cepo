//Copyright (c) 2026 CB604BL
#ifndef CEPO_RESULT_WHEN_ERROR_HPP
#define CEPO_RESULT_WHEN_ERROR_HPP

#include "cepo/callable/call.hpp"
#include "cepo/callable/call_result.hpp"
#include "cepo/callable/is_callable.hpp"
#include "cepo/configs/namespace_macro.h"
#include <type_traits>
#include <utility>

CEPO_NAMESPACE_START;

template<typename E>
class result_when_error
{
private:
	//const-only
	const E* error_handler;

public:
	result_when_error(const E* err) noexcept:
		error_handler(err)
	{}

	result_when_error(result_when_error&&) noexcept = default;
	result_when_error& operator=(result_when_error&&) noexcept = default;

	~result_when_error() noexcept = default;

	template<typename F>
	auto error(F&& callback) &&
		noexcept(is_nothrow_callable<F, const E&>::value)
		-> typename std::enable_if<
			is_callable<F, const E&>::value and
			std::is_void<
				typename call_result<F, const E&>::type>::value
		>::type
	{
		if (error_handler)
			call(std::forward<F>(callback), *error_handler);
	}
	
	result_when_error() = delete;
	result_when_error(const result_when_error&) = delete;
	result_when_error& operator=(const result_when_error&) = delete;
};

CEPO_NAMESPACE_END;

#endif //CEPO_RESULT_WHEN_ERROR_HPP
