//Copyright (c) 2026 CB604BL
//CEPO - C++ Eleven Patches Open-source
#ifndef CEPO_TYPE_TRAITS_IS_ONEWAY_EQUALITY_OPERABLE_WITH_HPP
#define CEPO_TYPE_TRAITS_IS_ONEWAY_EQUALITY_OPERABLE_WITH_HPP

#include "cepo/configs/namespace_macro.h"
#include "cepo/type_traits/bool_constant.hpp"
#include "cepo/type_traits/is_bool_testable.hpp"
#include <utility>

CEPO_IMPL_NAMESPACE_START;

template<typename T, typename U>
struct is_oneway_equality_operable_with
{
private:
	struct is_not_bool_testable_t
	{
		operator bool() const = delete;
		bool operator not() const = delete;
	};

	template<typename X, typename Y>
	static auto test(int) ->
		decltype(
			std::declval<X>() == std::declval<Y>());

	template<typename, typename>
	static is_not_bool_testable_t test(...);

public:
	static constexpr bool value =
		is_bool_testable<decltype(test<T, U>(1))>::value;
};

CEPO_IMPL_NAMESPACE_END;

CEPO_NAMESPACE_START;

template<typename T, typename U>
struct is_oneway_equality_operable_with
	: bool_constant<impl::is_oneway_equality_operable_with<T, U>::value>
{};

CEPO_NAMESPACE_END;

#endif //CEPO_TYPE_TRAITS_IS_ONEWAY_EQUALITY_OPERABLE_WITH_HPP
