//Copyright (c) 2026 CB604BL
//CEPO - C++ Eleven Patches Open-source
#ifndef CEPO_TYPE_TRAITS_IS_NOTHROW_CONVERTIBLE_HPP
#define CEPO_TYPE_TRAITS_IS_NOTHROW_CONVERTIBLE_HPP

#include "cepo/configs/namespace_macro.h"
#include <type_traits>
#include <utility>

CEPO_IMPL_NAMESPACE_START;

template<typename From, typename To,
	bool is_void = std::is_void<To>::value or std::is_void<From>::value>
struct is_nothrow_convertible
{
	static constexpr bool value = std::is_void<To>::value and std::is_void<From>::value;
};

template<typename From, typename To>
struct is_nothrow_convertible<From, To, false>
{
private:
	template<typename ToF>
	static void test_aux(ToF) noexcept;

	template<typename FromT, typename ToT>
	static auto test(int) ->
		decltype(
			//The static_cast here is only meant to reproduce the well-formedness check for a form like "To foo() {}".
			static_cast<ToT(*)()>(nullptr),
			std::integral_constant<
				bool,
				noexcept(test_aux<ToT>(std::declval<FromT>()))>{});

	template<typename...>
	static std::false_type test(...);

public:
	static constexpr bool value = decltype(test<From, To>(1))::value;
};

#if not defined(__cpp_inline_variables)
template<typename From, typename To, bool is_void>
constexpr bool is_nothrow_convertible<From, To, is_void>::value;

template<typename From, typename To>
constexpr bool is_nothrow_convertible<From, To, false>::value;
#endif //not defined(__cpp_inline_variables)

CEPO_IMPL_NAMESPACE_END;

CEPO_NAMESPACE_START;

template<typename From, typename To>
struct is_nothrow_convertible
{
	static constexpr bool value = impl::is_nothrow_convertible<From, To>::value;
};

#if not defined(__cpp_inline_variables)
template<typename From, typename To>
constexpr bool is_nothrow_convertible<From, To>::value;
#endif //not defined(__cpp_inline_variables)

CEPO_NAMESPACE_END;

#endif //CEPO_TYPE_TRAITS_IS_NOTHROW_CONVERTIBLE_HPP
