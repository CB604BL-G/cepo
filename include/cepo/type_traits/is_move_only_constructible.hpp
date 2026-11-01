//Copyright (c) 2026 CB604BL
//CEPO - C++ Eleven Patches Open-source
#ifndef CEPO_TYPE_TRAITS_IS_MOVE_ONLY_CONSTRUCTIBLE_HPP
#define CEPO_TYPE_TRAITS_IS_MOVE_ONLY_CONSTRUCTIBLE_HPP

#include "cepo/configs/namespace_macro.h"
#include <type_traits>

CEPO_NAMESPACE_START;

template<typename T>
struct is_move_only_constructible
	: std::integral_constant<
		bool,
		std::is_constructible<T, T&&>::value and
		not std::is_constructible<T, const T&>::value>
{};

CEPO_NAMESPACE_END;

#endif //CEPO_TYPE_TRAITS_IS_MOVE_ONLY_CONSTRUCTIBLE_HPP
