//Copyright (c) 2026 CB604BL
//CEPO - C++ Eleven Patches Open-source
#ifndef CEPO_TYPE_TRAITS_IS_ONEWAY_EQUALITY_COMPARISON_OPERABLE_WITH_HPP
#define CEPO_TYPE_TRAITS_IS_ONEWAY_EQUALITY_COMPARISON_OPERABLE_WITH_HPP

#include "cepo/configs/namespace_macro.h"
#include "cepo/type_traits/bool_constant.hpp"
#include "cepo/type_traits/is_oneway_equality_operable_with.hpp"
#include "cepo/type_traits/is_oneway_inequality_operable_with.hpp"
#include <type_traits>

CEPO_NAMESPACE_START;

//For this trait to yield true, both == and != for
//T and U must exist and be well-formed.
template<typename T, typename U>
struct is_oneway_equality_comparison_operable_with
	: bool_constant<
		is_oneway_equality_operable_with<
			const typename std::remove_reference<T>::type&,
			const typename std::remove_reference<U>::type&
		>::value and
		is_oneway_inequality_operable_with<
			const typename std::remove_reference<T>::type&,
			const typename std::remove_reference<U>::type&
		>::value>
{};

CEPO_NAMESPACE_END;

#endif //CEPO_TYPE_TRAITS_IS_ONEWAY_EQUALITY_COMPARISON_OPERABLE_WITH_HPP
