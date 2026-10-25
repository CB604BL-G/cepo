//Copyright (c) 2026 CB604BL
//CEPO - C++ Eleven Patches Open-source
#ifndef CEPO_RESULT_FACTORY_HPP
#define CEPO_RESULT_FACTORY_HPP

#include "cepo/configs/namespace_macro.h"
#include "cepo/macros/nodiscard.hpp"
#include "cepo/result/slots.hpp"
#include <type_traits>
#include <utility>

CEPO_NAMESPACE_START;

template<typename T, typename... Args>
CEPO_NODISCARD
typename std::enable_if<
	(std::is_void<T>::value and sizeof...(Args) == 0) or
	(std::is_constructible<T, Args...>::value),
	result_value_slot<T>
>::type
make_result_value(Args&&... args) noexcept
{
	return result_value_slot<T>(std::forward<Args>(args)...);
}

template<typename E, typename... Args>
CEPO_NODISCARD
typename std::enable_if<
	std::is_constructible<E, Args...>::value,
	result_error_slot<E>
>::type
make_result_error(Args&&... args) noexcept
{
	return result_error_slot<E>(std::forward<Args>(args)...);
}

CEPO_NAMESPACE_END;

#endif //CEPO_RESULT_FACTORY_HPP
