//Copyright (c) 2026 CB604BL
//CEPO - C++ Eleven Patches Open-source
#ifndef CEPO_TYPE_TRAITS_IS_ONEWAY_EQUALITY_COMPARISON_OPERABLE_WITH_HPP
#define CEPO_TYPE_TRAITS_IS_ONEWAY_EQUALITY_COMPARISON_OPERABLE_WITH_HPP

#include "cepo/configs/namespace_macro.h"
#include "cepo/type_traits/bool_constant.hpp"
#include "cepo/type_traits/is_bool_testable.hpp"
#include <type_traits>
#include <utility>

CEPO_IMPL_NAMESPACE_START;

template<typename T, typename U>
struct is_oneway_equality_comparison_operable_with
{
private:
	struct is_not_bool_testable_t
	{
		operator bool() const = delete;
		bool operator not() const = delete;
	};

	template<typename X, typename Y>
	static auto test_eq(int) ->
		decltype(
			std::declval<const typename std::remove_reference<X>::type&>()
			==
			std::declval<const typename std::remove_reference<Y>::type&>()
		);

	template<typename, typename>
	static is_not_bool_testable_t test_eq(...);

	
	template<typename X, typename Y>
	static auto test_not_eq(int) ->
		decltype(
			std::declval<const typename std::remove_reference<X>::type&>()
			!=
			std::declval<const typename std::remove_reference<Y>::type&>()
		);

	template<typename, typename>
	static is_not_bool_testable_t test_not_eq(...);

public:
	static constexpr bool value =
		is_bool_testable<decltype(test_eq<T, U>(1))>::value and
		is_bool_testable<decltype(test_not_eq<T, U>(1))>::value;
};

CEPO_IMPL_NAMESPACE_END;

CEPO_NAMESPACE_START;

//For this trait to yield true, both == and != for
//T and U must exist and be well-formed.
template<typename T, typename U>
struct is_oneway_equality_comparison_operable_with
	: bool_constant<impl::is_oneway_equality_comparison_operable_with<T, U>::value>
{};

CEPO_NAMESPACE_END;

#endif //CEPO_TYPE_TRAITS_IS_ONEWAY_EQUALITY_COMPARISON_OPERABLE_WITH_HPP
