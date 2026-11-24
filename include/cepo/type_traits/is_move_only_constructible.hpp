//Copyright (c) 2026 CB604BL
//CEPO - C++ Eleven Patches Open-source
#ifndef CEPO_TYPE_TRAITS_IS_MOVE_ONLY_CONSTRUCTIBLE_HPP
#define CEPO_TYPE_TRAITS_IS_MOVE_ONLY_CONSTRUCTIBLE_HPP

#include "cepo/configs/namespace_macro.hpp"
#include "cepo/type_traits/bool_constant.hpp"
#include <type_traits>

CEPO_NAMESPACE_START;

template<typename T>
struct is_move_only_constructible
	: bool_constant<
		std::is_constructible<T, T&&>::value and
		not std::is_constructible<T, const T&>::value>
{};

CEPO_NAMESPACE_END;

#endif //CEPO_TYPE_TRAITS_IS_MOVE_ONLY_CONSTRUCTIBLE_HPP
