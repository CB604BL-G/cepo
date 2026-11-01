//Copyright (c) 2026 CB604BL
//CEPO - C++ Eleven Patches Open-source
#ifndef CEPO_CALLABLE_IS_CALLABLE_HPP
#define CEPO_CALLABLE_IS_CALLABLE_HPP

#include "cepo/callable/call_category_macros.hpp"
#include "cepo/configs/namespace_macro.h"
#include "cepo/utility/priority_tag.hpp"
#include <type_traits>

CEPO_NAMESPACE_START;

#define BASIC_CALL(F, Args) \
	CEPO_CALLABLE_CALL_CATEGORY_EXPR_BASIC_CALL(F, Args)

#define POINTER_TO_MEMBER_CALL_BY_OBJECT(F, Obj, Args) \
	CEPO_CALLABLE_CALL_CATEGORY_EXPR_POINTER_TO_MEMBER_CALL_BY_OBJECT(F, Obj, Args)

#define POINTER_TO_MEMBER_CALL_BY_POINTER(F, Ptr, Args) \
	CEPO_CALLABLE_CALL_CATEGORY_EXPR_POINTER_TO_MEMBER_CALL_BY_POINTER(F, Ptr, Args)

template<typename F, typename... Args>
struct is_callable
{
private:
	template<typename F_T, typename... Args_T>
	static auto test(priority_tag<2>) ->
		decltype(BASIC_CALL(F_T, Args_T),
		std::true_type{});

	template<typename F_O, typename Obj, typename... Args_O>
	static auto test(priority_tag<1>) ->
		decltype(
			POINTER_TO_MEMBER_CALL_BY_OBJECT(F_O, Obj, Args_O),
			std::true_type{});

	template<typename F_P, typename Ptr, typename... Args_P>
	static auto test(priority_tag<0>) ->
		decltype(
			POINTER_TO_MEMBER_CALL_BY_POINTER(F_P, Ptr, Args_P),
			std::true_type{});

	template<typename...>
	static std::false_type test(...);

public:
	static constexpr bool value = decltype(test<F, Args...>(priority_tag<2>{}))::value;
};

//Detects whether F(Args...) is both callable and noexcept.
//false does not distinguish "not callable" from "callable but throwing".

//In C++11/14, noexcept is not part of the function type, so the noexcept
//specification of function references / function pointers / member-function
//pointers cannot be observed through the type system (only functors and
//lambdas are reliable). C++17 introduced noexcept function types via
//__cpp_noexcept_function_type; only then are the member-pointer overloads
//below compiled and able to participate in overload resolution. On older
//standards they are omitted.
template<typename F, typename... Args>
struct is_nothrow_callable
{
private:
	template<typename F_T, typename... Args_T>
	static auto test(priority_tag<2>) ->
		std::integral_constant<bool, noexcept(BASIC_CALL(F_T, Args_T))>;

	template<typename...>
	static std::false_type test(...);

#if defined(__cpp_noexcept_function_type)

	template<typename F_O, typename Obj, typename... Args_O>
	static auto test(priority_tag<1>) ->
		std::integral_constant<bool, noexcept(POINTER_TO_MEMBER_CALL_BY_OBJECT(F_O, Obj, Args_O))>;

	
	template<typename F_P, typename Ptr, typename... Args_P>
	static auto test(priority_tag<0>) ->
		std::integral_constant<bool, noexcept(POINTER_TO_MEMBER_CALL_BY_POINTER(F_P, Ptr, Args_P))>;

#endif

public:
	static constexpr bool value = decltype(test<F, Args...>(priority_tag<2>{}))::value;
};

template<typename F, typename... Args>
constexpr bool is_callable<F, Args...>::value;

template<typename F, typename... Args>
constexpr bool is_nothrow_callable<F, Args...>::value;

#undef BASIC_CALL
#undef POINTER_TO_MEMBER_CALL_BY_OBJECT
#undef POINTER_TO_MEMBER_CALL_BY_POINTER

CEPO_NAMESPACE_END;

#endif //CEPO_CALLABLE_IS_CALLABLE_HPP
