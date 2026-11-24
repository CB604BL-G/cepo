//Copyright (c) 2026 CB604BL
//CEPO - C++ Eleven Patches Open-source
#ifndef CEPO_TYPE_TRAITS_IS_BOOL_TESTABLE_HPP
#define CEPO_TYPE_TRAITS_IS_BOOL_TESTABLE_HPP

#include "cepo/configs/namespace_macro.hpp"
#include "cepo/type_traits/bool_constant.hpp"
#include <type_traits>
#include <utility>

CEPO_IMPL_NAMESPACE_START;

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

CEPO_IMPL_NAMESPACE_END;

CEPO_NAMESPACE_START;

template<typename T>
struct is_bool_testable
	: bool_constant<impl::is_bool_testable<T>::value>
{};

CEPO_NAMESPACE_END;

#endif //CEPO_TYPE_TRAITS_IS_BOOL_TESTABLE_HPP
