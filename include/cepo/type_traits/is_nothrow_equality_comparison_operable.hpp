//Copyright (c) 2026 CB604BL
//CEPO - C++ Eleven Patches Open-source
#ifndef CEPO_TYPE_TRAITS_IS_NOTHROW_EQUALITY_COMPARISON_OPERABLE_HPP
#define CEPO_TYPE_TRAITS_IS_NOTHROW_EQUALITY_COMPARISON_OPERABLE_HPP

#include "cepo/configs/namespace_macro.hpp"
#include "cepo/type_traits/bool_constant.hpp"
#include "cepo/type_traits/is_nothrow_oneway_equality_comparison_operable_with.hpp"

CEPO_NAMESPACE_START;

template<typename T>
struct is_nothrow_equality_comparison_operable
	: bool_constant<is_nothrow_oneway_equality_comparison_operable_with<T, T>::value>
{};

CEPO_NAMESPACE_END;

#endif //CEPO_TYPE_TRAITS_IS_NOTHROW_EQUALITY_COMPARISON_OPERABLE_HPP
