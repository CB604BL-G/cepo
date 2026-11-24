//Copyright (c) 2026 CB604BL
//CEPO - C++ Eleven Patches Open-source
#ifndef CEPO_TYPE_TRAITS_NEGATION_HPP
#define CEPO_TYPE_TRAITS_NEGATION_HPP

#include "cepo/configs/namespace_macro.hpp"
#include "cepo/type_traits/bool_constant.hpp"

CEPO_NAMESPACE_START;

template<typename B>
struct negation
	: bool_constant<not bool(B::value)>
{};

CEPO_NAMESPACE_END;

#endif //CEPO_TYPE_TRAITS_NEGATION_HPP
