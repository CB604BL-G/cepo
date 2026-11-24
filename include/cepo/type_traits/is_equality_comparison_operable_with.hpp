//Copyright (c) 2026 CB604BL
//CEPO - C++ Eleven Patches Open-source
#ifndef CEPO_TYPE_TRAITS_IS_EQUALITY_COMPARISON_OPERABLE_WITH_HPP
#define CEPO_TYPE_TRAITS_IS_EQUALITY_COMPARISON_OPERABLE_WITH_HPP

#include "cepo/configs/namespace_macro.hpp"
#include "cepo/type_traits/bool_constant.hpp"
#include "cepo/type_traits/is_oneway_equality_comparison_operable_with.hpp"

CEPO_NAMESPACE_START;

template<typename T, typename U>
struct is_equality_comparison_operable_with
	: bool_constant<
		is_oneway_equality_comparison_operable_with<T, U>::value and
		is_oneway_equality_comparison_operable_with<U, T>::value>
{};

CEPO_NAMESPACE_END;

#endif //CEPO_TYPE_TRAITS_IS_EQUALITY_COMPARISON_OPERABLE_WITH_HPP
