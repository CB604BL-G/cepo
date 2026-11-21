//Copyright (c) 2026 CB604BL
//CEPO - C++ Eleven Patches Open-source
#ifndef CEPO_RESULT_WHEN_EXCEPTIONAL_HPP
#define CEPO_RESULT_WHEN_EXCEPTIONAL_HPP

#include "cepo/callable/call.hpp"
#include "cepo/callable/is_callable.hpp"
#include "cepo/callable/is_nothrow_callable.hpp"
#include "cepo/configs/namespace_macro.h"
#include <exception>
#include <type_traits>
#include <utility>

CEPO_NAMESPACE_START;

class result_when_exceptional
{
private:
	const std::exception_ptr* exception_ptr;

public:
	result_when_exceptional(const std::exception_ptr* exception) noexcept;
	result_when_exceptional(result_when_exceptional&&) noexcept = default;
	result_when_exceptional& operator=(result_when_exceptional&&) noexcept = default;
	~result_when_exceptional() noexcept = default;

	//The callback is invoked with *exception_ptr, that is a
	//const std::exception_ptr&, so that is the type the traits have to
	//probe with. A callback taking std::exception_ptr by value is still
	//accepted and receives a copy; one taking std::exception_ptr&& or
	//std::exception_ptr& is rejected here instead of failing inside the
	//body.
	template<typename F>
	auto exceptional(F&& callback) &&
		noexcept(is_nothrow_callable<F, const std::exception_ptr&>::value)
		-> typename std::enable_if<
			is_callable<F, const std::exception_ptr&>::value
		>::type
	{
		if (exception_ptr)
			call(std::forward<F>(callback), *exception_ptr);
	}

	void ignore_exceptional() && noexcept;

	result_when_exceptional() = delete;
	result_when_exceptional(const result_when_exceptional&) = delete;
	result_when_exceptional& operator=(const result_when_exceptional&) = delete;
};

CEPO_NAMESPACE_END;

#endif //CEPO_RESULT_WHEN_EXCEPTIONAL_HPP
