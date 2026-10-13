//Copyright (c) 2026 CB604BL
#ifndef CEPO_TYPE_TRAITS_DISJUNCTION_HPP
#define CEPO_TYPE_TRAITS_DISJUNCTION_HPP

#include "configs/namespace_macro.h"
#include <type_traits>

CEPO_NAMESPACE_START;

template<typename...>
struct disjunction : std::false_type{};

template<typename B1>
struct disjunction<B1> : B1{};

template<typename B1, typename... Bs>
struct disjunction<B1, Bs...>
	: std::conditional<
		bool(B1::value),
		B1,
		disjunction<Bs...>>::type
{};

CEPO_NAMESPACE_END;

#endif //CEPO_TYPE_TRAITS_DISJUNCTION_HPP
