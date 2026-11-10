//Copyright (c) 2026 CB604BL
//CEPO - C++ Eleven Patches Open-source
#ifndef CEPO_CALLABLE_HAS_UNOVERLOADED_OPERATOR_HPP
#define CEPO_CALLABLE_HAS_UNOVERLOADED_OPERATOR_HPP

#include "cepo/configs/namespace_macro.h"
#include <type_traits>

//You can also think of
//the template operator as a form of overloading
CEPO_IMPL_NAMESPACE_START;

template<typename T>
struct has_unoverloaded_operator
{
private:
	template<typename U>
	static auto test(int) ->
		decltype(&U::operator(),
		std::true_type{});

	template<typename U>
	static std::false_type test(...);

public:
	static constexpr bool value = decltype(test<T>(1))::value;
};

template<typename T>
constexpr bool has_unoverloaded_operator<T>::value;

CEPO_IMPL_NAMESPACE_END;

#endif //CEPO_CALLABLE_HAS_UNOVERLOADED_OPERATOR_HPP
