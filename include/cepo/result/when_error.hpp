//Copyright (c) 2026 CB604BL
//CEPO - C++ Eleven Patches Open-source
#ifndef CEPO_RESULT_WHEN_ERROR_HPP
#define CEPO_RESULT_WHEN_ERROR_HPP

#include "cepo/callable/call.hpp"
#include "cepo/callable/call_result.hpp"
#include "cepo/callable/is_callable.hpp"
#include "cepo/configs/namespace_macro.h"
#include <type_traits>
#include <utility>

CEPO_NAMESPACE_START;

//At all times, result_when_error is only ever returned by result::ok
//as a temporary object. Do not attempt to store it; the safe approach
//is to call its member functions directly within the ok call chain.
//This is because it holds a pointer to the result's error object. If the
//result is long-lived, that is fine; but if the result is itself a
//temporary object, you are done for. In addition, even if you store it,
//the rvalue ref-qualifier (&&) on the member functions of result_when_error
//will prevent such use. However, some people may always find hacky ways
//to circumvent these restrictions, and we are not responsible for that.
//To state it once more: the safe approach is to consume it immediately
//within the call chain.
template<typename E>
class result_when_error
{
private:
	//const-only
	const E* error_ptr;

public:
	result_when_error(const E* err) noexcept:
		error_ptr(err)
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
		if (error_ptr)
			call(std::forward<F>(callback), *error_ptr);
	}

	void ignore_error() && noexcept {}
	
	result_when_error() = delete;
	result_when_error(const result_when_error&) = delete;
	result_when_error& operator=(const result_when_error&) = delete;
};

CEPO_NAMESPACE_END;

#endif //CEPO_RESULT_WHEN_ERROR_HPP
