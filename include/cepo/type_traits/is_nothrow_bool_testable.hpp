//Copyright (c) 2026 CB604BL
//CEPO - C++ Eleven Patches Open-source
#ifndef CEPO_TYPE_TRAITS_IS_NOTHROW_BOOL_TESTABLE_HPP
#define CEPO_TYPE_TRAITS_IS_NOTHROW_BOOL_TESTABLE_HPP

#include "cepo/configs/namespace_macro.h"
#include "cepo/type_traits/bool_constant.hpp"
#include <type_traits>
#include <utility>

CEPO_NAMESPACE_START;

template<typename T>
struct is_nothrow_bool_testable
{
private:
	template<typename X>
	static auto can_nothrow_construct_bool(int) ->
		decltype(
			bool_constant<noexcept((bool(std::declval<X>())))>{});

	template<typename>
	static std::false_type can_nothrow_construct_bool(...);

	template<typename X>
	static auto nothrow_not_operable(int) ->
		decltype(
			bool_constant<noexcept((not std::declval<X>()))>{});

	template<typename>
	static std::false_type nothrow_not_operable(...);

public:
	static constexpr bool value =
		decltype(can_nothrow_construct_bool<T>(1))::value and
		decltype(nothrow_not_operable<T>(1))::value;
};

#if not defined(__cpp_inline_variables)
template<typename T>
constexpr bool is_nothrow_bool_testable<T>::value;
#endif //not defined(__cpp_inline_variables)

CEPO_NAMESPACE_END;

#endif //CEPO_TYPE_TRAITS_IS_NOTHROW_BOOL_TESTABLE_HPP
