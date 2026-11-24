//Copyright (c) 2026 CB604BL
//CEPO - C++ Eleven Patches Open-source
#ifndef CEPO_TYPE_TRAITS_IS_NOTHROW_ONEWAY_EQUALITY_OPERABLE_WITH_HPP
#define CEPO_TYPE_TRAITS_IS_NOTHROW_ONEWAY_EQUALITY_OPERABLE_WITH_HPP

#include "cepo/configs/namespace_macro.hpp"
#include "cepo/type_traits/bool_constant.hpp"
#include "cepo/type_traits/is_nothrow_bool_testable.hpp"
#include <type_traits>
#include <utility>

CEPO_IMPL_NAMESPACE_START;

template<typename T, typename U>
struct is_nothrow_oneway_equality_operable_with
{
private:
	struct is_not_bool_testable_t
	{
		operator bool() const = delete;
		bool operator not() const = delete;
	};

	template<typename X, typename Y>
	static auto test_expr(int) ->
		decltype(std::declval<X>() == std::declval<Y>());

	template<typename, typename>
	static is_not_bool_testable_t test_expr(...);

	template<typename X, typename Y>
	static auto test_noexcept(int) ->
		bool_constant<noexcept(std::declval<X>() == std::declval<Y>())>;

	template<typename, typename>
	static std::false_type test_noexcept(...);

public:
	static constexpr bool value =
		is_nothrow_bool_testable<decltype(test_expr<T, U>(1))>::value and
		decltype(test_noexcept<T, U>(1))::value;
};

CEPO_IMPL_NAMESPACE_END;

CEPO_NAMESPACE_START;

template<typename T, typename U>
struct is_nothrow_oneway_equality_operable_with
	: bool_constant<impl::is_nothrow_oneway_equality_operable_with<T, U>::value>
{};

CEPO_NAMESPACE_END;

#endif //CEPO_TYPE_TRAITS_IS_NOTHROW_ONEWAY_EQUALITY_OPERABLE_WITH_HPP
