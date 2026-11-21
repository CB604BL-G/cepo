//Copyright (c) 2026 CB604BL
//CEPO - C++ Eleven Patches Open-source
#include "cepo/result/when_exceptional.hpp"
#include "cepo/configs/namespace_macro.h"
#include <exception>

CEPO_NAMESPACE_START;

result_when_exceptional::result_when_exceptional(const std::exception_ptr* exception) noexcept:
	exception_ptr(exception)
{}

void result_when_exceptional::ignore_exceptional() && noexcept {}

CEPO_NAMESPACE_END;
