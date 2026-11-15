//Copyright (c) 2026 CB604BL
//CEPO - C++ Eleven Patches Open-source
#ifndef CEPO_TYPE_TRAITS_IS_BOOL_TESTABLE_HPP
#define CEPO_TYPE_TRAITS_IS_BOOL_TESTABLE_HPP

#include "cepo/configs/namespace_macro.h"
#include <type_traits>
#include <utility>

CEPO_NAMESPACE_START;

template<typename T>
struct is_bool_testable
{
private:
	template<typename X>
	static auto can_construct_bool(int) ->
		decltype(
			(bool(std::declval<X>())),
			std::true_type{}
		);

	template<typename>
	static std::false_type can_construct_bool(...);

	template<typename X>
	static auto not_operable(int) ->
		decltype(
			(not std::declval<X>()),
			std::true_type{}
		);

	template<typename>
	static std::false_type not_operable(...);

public:
	static constexpr bool value =
		decltype(can_construct_bool<T>(1))::value and
		decltype(not_operable<T>(1))::value;
};

#if not defined(__cpp_inline_variables)
template<typename T>
constexpr bool is_bool_testable<T>::value;
#endif //not defined(__cpp_inline_variables)

CEPO_NAMESPACE_END;

#endif //CEPO_TYPE_TRAITS_IS_BOOL_TESTABLE_HPP
