//Copyright (c) 2026 CB604BL
#ifndef CEPO_TYPE_TRAITS_CONJUNCTION_HPP
#define CEPO_TYPE_TRAITS_CONJUNCTION_HPP

#include "cepo/configs/namespace_macro.h"
#include <type_traits>

CEPO_NAMESPACE_START;

template<typename...>
struct conjunction : std::true_type{};

template<typename B1>
struct conjunction<B1> : B1 {};

template<typename B1, typename... Bs>
struct conjunction<B1, Bs...>
	: std::conditional<
		bool(B1::value),
		conjunction<Bs...>,
		B1>::type
{};

CEPO_NAMESPACE_END;

#endif //CEPO_TYPE_TRAITS_CONJUNCTION_HPP
