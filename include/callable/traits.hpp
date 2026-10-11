//Copyright (c) 2026 CB604BL
#ifndef CEPO_CALLABLE_TRAITS_HPP
#define CEPO_CALLABLE_TRAITS_HPP

#include "callable/arg_tuple.hpp"
#include "callable/has_unoverloaded_operator.hpp"
#include "callable/t_tags.hpp"
#include "configs/namespace_macro.h"
#include "utility/always_false.hpp"
#include <cstddef>
#include <type_traits>

CEPO_NAMESPACE_START

template<typename... Ts>
struct callable_traits
{
	static_assert(
		always_false<Ts...>::value,
		"cepo::callable_traits: Bro, how the hell did you match up with the master template?"
	);
};

/*For base function type*/
template<typename Ret, typename... Args>
struct callable_traits<Ret(Args...)>
{
	//category
	static constexpr bool is_c_style_variadic_function = false;
	static constexpr bool is_pointer_to_function = false;
	static constexpr bool is_pointer_to_member_function = false;

	//qualifiers
	static constexpr bool is_const_qualified = false;
	static constexpr bool is_volatile_qualified = false;
	static constexpr bool is_ref_qualified = false;
	static constexpr bool is_lvalue_ref_qualified = false;
	static constexpr bool is_rvalue_ref_qualified = false;
	
	using fixed_arg_types = arg_tuple<Args...>;

	static constexpr std::size_t fixed_arg_count = fixed_arg_types::size;

	template<std::size_t index>
	using arg_type_at = typename fixed_arg_types::template get_type_at<index>::type;

	using return_type = Ret;

	//The void here is a sentinel type
	//meaning there's no class type
	//This attribute is reserved for member function pointers
	using class_type = void;
};

/*For abominable function type*/
//only-cv qualified
template<typename Ret, typename... Args>
struct callable_traits<Ret(Args...) const>
	: callable_traits<Ret(Args...)>
{
	static constexpr bool is_const_qualified = true;
};

template<typename Ret, typename... Args>
struct callable_traits<Ret(Args...) volatile>
	: callable_traits<Ret(Args...)>
{
	static constexpr bool is_volatile_qualified = true;
};

template<typename Ret, typename... Args>
struct callable_traits<Ret(Args...) const volatile>
	: callable_traits<Ret(Args...) const>
{
	static constexpr bool is_volatile_qualified = true;
};

//lvalue-ref qualified
template<typename Ret, typename... Args>
struct callable_traits<Ret(Args...) &>
	: callable_traits<Ret(Args...)>
{
	static constexpr bool is_ref_qualified = true;
	static constexpr bool is_lvalue_ref_qualified = true;
};

template<typename Ret, typename... Args>
struct callable_traits<Ret(Args...) const &>
	: callable_traits<Ret(Args...) const>
{
	static constexpr bool is_ref_qualified = true;
	static constexpr bool is_lvalue_ref_qualified = true;
};

template<typename Ret, typename... Args>
struct callable_traits<Ret(Args...) volatile &>
	: callable_traits<Ret(Args...) volatile>
{
	static constexpr bool is_ref_qualified = true;
	static constexpr bool is_lvalue_ref_qualified = true;
};

template<typename Ret, typename... Args>
struct callable_traits<Ret(Args...) const volatile &>
	: callable_traits<Ret(Args...) const volatile>
{
	static constexpr bool is_ref_qualified = true;
	static constexpr bool is_lvalue_ref_qualified = true;
};

//rvalue-ref qualified
template<typename Ret, typename... Args>
struct callable_traits<Ret(Args...) &&>
	: callable_traits<Ret(Args...)>
{
	static constexpr bool is_ref_qualified = true;
	static constexpr bool is_rvalue_ref_qualified = true;
};

template<typename Ret, typename... Args>
struct callable_traits<Ret(Args...) const &&>
	: callable_traits<Ret(Args...) const>
{
	static constexpr bool is_ref_qualified = true;
	static constexpr bool is_rvalue_ref_qualified = true;
};

template<typename Ret, typename... Args>
struct callable_traits<Ret(Args...) volatile &&>
	: callable_traits<Ret(Args...) volatile>
{
	static constexpr bool is_ref_qualified = true;
	static constexpr bool is_rvalue_ref_qualified = true;
};

template<typename Ret, typename... Args>
struct callable_traits<Ret(Args...) const volatile &&>
	: callable_traits<Ret(Args...) const volatile>
{
	static constexpr bool is_ref_qualified = true;
	static constexpr bool is_rvalue_ref_qualified = true;
};

/*For c-style variadic function type*/
template<typename Ret, typename... Args>
struct callable_traits<Ret(Args..., ...)>
	: callable_traits<Ret(Args...)>
{
	static constexpr bool is_c_style_variadic_function = true;
};

//only-cv qualified
template<typename Ret, typename... Args>
struct callable_traits<Ret(Args..., ...) const>
	: callable_traits<Ret(Args...) const>
{
	static constexpr bool is_c_style_variadic_function = true;
};

template<typename Ret, typename... Args>
struct callable_traits<Ret(Args..., ...) volatile>
	: callable_traits<Ret(Args...) volatile>
{
	static constexpr bool is_c_style_variadic_function = true;
};

template<typename Ret, typename... Args>
struct callable_traits<Ret(Args..., ...) const volatile>
	: callable_traits<Ret(Args...) const volatile>
{
	static constexpr bool is_c_style_variadic_function = true;
};

//lvalue-ref qualified
template<typename Ret, typename... Args>
struct callable_traits<Ret(Args..., ...) &>
	: callable_traits<Ret(Args...) &>
{
	static constexpr bool is_c_style_variadic_function = true;
};

template<typename Ret, typename... Args>
struct callable_traits<Ret(Args..., ...) const &>
	: callable_traits<Ret(Args...) const &>
{
	static constexpr bool is_c_style_variadic_function = true;
};

template<typename Ret, typename... Args>
struct callable_traits<Ret(Args..., ...) volatile &>
	: callable_traits<Ret(Args...) volatile &>
{
	static constexpr bool is_c_style_variadic_function = true;
};

template<typename Ret, typename... Args>
struct callable_traits<Ret(Args..., ...) const volatile &>
	: callable_traits<Ret(Args...) const volatile &>
{
	static constexpr bool is_c_style_variadic_function = true;
};

//rvalue-ref qualified
template<typename Ret, typename... Args>
struct callable_traits<Ret(Args..., ...) &&>
	: callable_traits<Ret(Args...) &&>
{
	static constexpr bool is_c_style_variadic_function = true;
};

template<typename Ret, typename... Args>
struct callable_traits<Ret(Args..., ...) const &&>
	: callable_traits<Ret(Args...) const &&>
{
	static constexpr bool is_c_style_variadic_function = true;
};

template<typename Ret, typename... Args>
struct callable_traits<Ret(Args..., ...) volatile &&>
	: callable_traits<Ret(Args...) volatile &&>
{
	static constexpr bool is_c_style_variadic_function = true;
};

template<typename Ret, typename... Args>
struct callable_traits<Ret(Args..., ...) const volatile &&>
	: callable_traits<Ret(Args...) const volatile &&>
{
	static constexpr bool is_c_style_variadic_function = true;
};

/*For pointer to function type*/
//the type cannot be with any function-qualifiers
template<typename Ret, typename... Args>
struct callable_traits<Ret(*)(Args...)>
	: callable_traits<Ret(Args...)>
{
	static constexpr bool is_pointer_to_function = true;
};

template<typename Ret, typename... Args>
struct callable_traits<Ret(*)(Args..., ...)>
	: callable_traits<Ret(*)(Args...)>
{
	static constexpr bool is_c_style_variadic_function = true;
};

/*For pointer to member function type*/
template<typename T, typename Ret, typename... Args>
struct callable_traits<Ret(T::*)(Args...)>
	: callable_traits<Ret(Args...)>
{
	static constexpr bool is_pointer_to_member_function = true;
	using class_type = T;
};

//only-cv qualified
template<typename T, typename Ret, typename... Args>
struct callable_traits<Ret(T::*)(Args...) const>
	: callable_traits<Ret(Args...) const>
{
	static constexpr bool is_pointer_to_member_function = true;
	using class_type = T;
};

template<typename T, typename Ret, typename... Args>
struct callable_traits<Ret(T::*)(Args...) volatile>
	: callable_traits<Ret(Args...) volatile>
{
	static constexpr bool is_pointer_to_member_function = true;
	using class_type = T;
};

template<typename T, typename Ret, typename... Args>
struct callable_traits<Ret(T::*)(Args...) const volatile>
	: callable_traits<Ret(Args...) const volatile>
{
	static constexpr bool is_pointer_to_member_function = true;
	using class_type = T;
};

//lvalue-ref qualified
template<typename T, typename Ret, typename... Args>
struct callable_traits<Ret(T::*)(Args...) &>
	: callable_traits<Ret(Args...) &>
{
	static constexpr bool is_pointer_to_member_function = true;
	using class_type = T;
};

template<typename T, typename Ret, typename... Args>
struct callable_traits<Ret(T::*)(Args...) const &>
	: callable_traits<Ret(Args...) const &>
{
	static constexpr bool is_pointer_to_member_function = true;
	using class_type = T;
};

template<typename T, typename Ret, typename... Args>
struct callable_traits<Ret(T::*)(Args...) volatile &>
	: callable_traits<Ret(Args...) volatile &>
{
	static constexpr bool is_pointer_to_member_function = true;
	using class_type = T;
};

template<typename T, typename Ret, typename... Args>
struct callable_traits<Ret(T::*)(Args...) const volatile &>
	: callable_traits<Ret(Args...) const volatile &>
{
	static constexpr bool is_pointer_to_member_function = true;
	using class_type = T;
};

//rvalue-ref qualified
template<typename T, typename Ret, typename... Args>
struct callable_traits<Ret(T::*)(Args...) &&>
	: callable_traits<Ret(Args...) &&>
{
	static constexpr bool is_pointer_to_member_function = true;
	using class_type = T;
};

template<typename T, typename Ret, typename... Args>
struct callable_traits<Ret(T::*)(Args...) const &&>
	: callable_traits<Ret(Args...) const &&>
{
	static constexpr bool is_pointer_to_member_function = true;
	using class_type = T;
};

template<typename T, typename Ret, typename... Args>
struct callable_traits<Ret(T::*)(Args...) volatile &&>
	: callable_traits<Ret(Args...) volatile &&>
{
	static constexpr bool is_pointer_to_member_function = true;
	using class_type = T;
};

template<typename T, typename Ret, typename... Args>
struct callable_traits<Ret(T::*)(Args...) const volatile &&>
	: callable_traits<Ret(Args...) const volatile &&>
{
	static constexpr bool is_pointer_to_member_function = true;
	using class_type = T;
};

/*For c-style variadic pointer to member function type*/
template<typename T, typename Ret, typename... Args>
struct callable_traits<Ret(T::*)(Args..., ...)>
	: callable_traits<Ret(T::*)(Args...)>
{
	static constexpr bool is_c_style_variadic_function = true;
};

//only-cv qualified
template<typename T, typename Ret, typename... Args>
struct callable_traits<Ret(T::*)(Args..., ...) const>
	: callable_traits<Ret(T::*)(Args...) const>
{
	static constexpr bool is_c_style_variadic_function = true;
};

template<typename T, typename Ret, typename... Args>
struct callable_traits<Ret(T::*)(Args..., ...) volatile>
	: callable_traits<Ret(T::*)(Args...) volatile>
{
	static constexpr bool is_c_style_variadic_function = true;
};

template<typename T, typename Ret, typename... Args>
struct callable_traits<Ret(T::*)(Args..., ...) const volatile>
	: callable_traits<Ret(T::*)(Args...) const volatile>
{
	static constexpr bool is_c_style_variadic_function = true;
};

//lvalue-ref qualified
template<typename T, typename Ret, typename... Args>
struct callable_traits<Ret(T::*)(Args..., ...) &>
	: callable_traits<Ret(T::*)(Args...) &>
{
	static constexpr bool is_c_style_variadic_function = true;
};

template<typename T, typename Ret, typename... Args>
struct callable_traits<Ret(T::*)(Args..., ...) const &>
	: callable_traits<Ret(T::*)(Args...) const &>
{
	static constexpr bool is_c_style_variadic_function = true;
};

template<typename T, typename Ret, typename... Args>
struct callable_traits<Ret(T::*)(Args..., ...) volatile &>
	: callable_traits<Ret(T::*)(Args...) volatile &>
{
	static constexpr bool is_c_style_variadic_function = true;
};

template<typename T, typename Ret, typename... Args>
struct callable_traits<Ret(T::*)(Args..., ...) const volatile &>
	: callable_traits<Ret(T::*)(Args...) const volatile &>
{
	static constexpr bool is_c_style_variadic_function = true;
};

//rvalue-ref qualified
template<typename T, typename Ret, typename... Args>
struct callable_traits<Ret(T::*)(Args..., ...) &&>
	: callable_traits<Ret(T::*)(Args...) &&>
{
	static constexpr bool is_c_style_variadic_function = true;
};

template<typename T, typename Ret, typename... Args>
struct callable_traits<Ret(T::*)(Args..., ...) const &&>
	: callable_traits<Ret(T::*)(Args...) const &&>
{
	static constexpr bool is_c_style_variadic_function = true;
};

template<typename T, typename Ret, typename... Args>
struct callable_traits<Ret(T::*)(Args..., ...) volatile &&>
	: callable_traits<Ret(T::*)(Args...) volatile &&>
{
	static constexpr bool is_c_style_variadic_function = true;
};

template<typename T, typename Ret, typename... Args>
struct callable_traits<Ret(T::*)(Args..., ...) const volatile &&>
	: callable_traits<Ret(T::*)(Args...) const volatile &&>
{
	static constexpr bool is_c_style_variadic_function = true;
};

/*For class types with operator() (functors, lambdas, etc.)*/
template<typename F>
struct callable_traits<F>
	: callable_traits<
		typename std::conditional<std::is_class<F>::value,
			callable::is_class_type, callable::is_not_class_type
		>::type,
		F
	>
{};

template<typename F>
struct callable_traits<callable::is_class_type, F>
	: callable_traits<
		typename std::conditional<callable::has_unoverloaded_operator<F>::value,
			callable::is_with_a_unique_operator, callable::is_not_with_a_unique_operator
		>::type,
		F
	>
{};

template<typename F>
struct callable_traits<callable::is_not_class_type, F>
{
	static_assert(
		always_false<F>::value,
		"cepo::callable_traits: T is not a callable type");
};

template<typename F>
struct callable_traits<callable::is_not_with_a_unique_operator, F>
{
	static_assert(
		always_false<F>::value,
		"cepo::callable_traits: T is not a callable type with a unique operator()");
};

template<typename F>
struct callable_traits<callable::is_with_a_unique_operator, F>
	: callable_traits<decltype(&F::operator())>
{};

/*The following are all out-of-class definitions*/
/*For base function type*/
template<typename Ret, typename... Args>
constexpr bool callable_traits<Ret(Args...)>::is_c_style_variadic_function;
template<typename Ret, typename... Args>
constexpr bool callable_traits<Ret(Args...)>::is_pointer_to_function;
template<typename Ret, typename... Args>
constexpr bool callable_traits<Ret(Args...)>::is_pointer_to_member_function;

template<typename Ret, typename... Args>
constexpr bool callable_traits<Ret(Args...)>::is_const_qualified;
template<typename Ret, typename... Args>
constexpr bool callable_traits<Ret(Args...)>::is_volatile_qualified;
template<typename Ret, typename... Args>
constexpr bool callable_traits<Ret(Args...)>::is_ref_qualified;
template<typename Ret, typename... Args>
constexpr bool callable_traits<Ret(Args...)>::is_lvalue_ref_qualified;
template<typename Ret, typename... Args>
constexpr bool callable_traits<Ret(Args...)>::is_rvalue_ref_qualified;

template<typename Ret, typename... Args>
constexpr std::size_t callable_traits<Ret(Args...)>::fixed_arg_count;

/*For abominable function type*/
//only-cv qualified
template<typename Ret, typename... Args>
constexpr bool callable_traits<Ret(Args...) const>::is_const_qualified;
template<typename Ret, typename... Args>
constexpr bool callable_traits<Ret(Args...) volatile>::is_volatile_qualified;
template<typename Ret, typename... Args>
constexpr bool callable_traits<Ret(Args...) const volatile>::is_volatile_qualified;

//lvalue-ref qualified
template<typename Ret, typename... Args>
constexpr bool callable_traits<Ret(Args...) &>::is_ref_qualified;
template<typename Ret, typename... Args>
constexpr bool callable_traits<Ret(Args...) &>::is_lvalue_ref_qualified;

template<typename Ret, typename... Args>
constexpr bool callable_traits<Ret(Args...) const &>::is_ref_qualified;
template<typename Ret, typename... Args>
constexpr bool callable_traits<Ret(Args...) const &>::is_lvalue_ref_qualified;

template<typename Ret, typename... Args>
constexpr bool callable_traits<Ret(Args...) volatile &>::is_ref_qualified;
template<typename Ret, typename... Args>
constexpr bool callable_traits<Ret(Args...) volatile &>::is_lvalue_ref_qualified;

template<typename Ret, typename... Args>
constexpr bool callable_traits<Ret(Args...) const volatile &>::is_ref_qualified;
template<typename Ret, typename... Args>
constexpr bool callable_traits<Ret(Args...) const volatile &>::is_lvalue_ref_qualified;

//rvalue-ref qualified
template<typename Ret, typename... Args>
constexpr bool callable_traits<Ret(Args...) &&>::is_ref_qualified;
template<typename Ret, typename... Args>
constexpr bool callable_traits<Ret(Args...) &&>::is_rvalue_ref_qualified;

template<typename Ret, typename... Args>
constexpr bool callable_traits<Ret(Args...) const &&>::is_ref_qualified;
template<typename Ret, typename... Args>
constexpr bool callable_traits<Ret(Args...) const &&>::is_rvalue_ref_qualified;

template<typename Ret, typename... Args>
constexpr bool callable_traits<Ret(Args...) volatile &&>::is_ref_qualified;
template<typename Ret, typename... Args>
constexpr bool callable_traits<Ret(Args...) volatile &&>::is_rvalue_ref_qualified;

template<typename Ret, typename... Args>
constexpr bool callable_traits<Ret(Args...) const volatile &&>::is_ref_qualified;
template<typename Ret, typename... Args>
constexpr bool callable_traits<Ret(Args...) const volatile &&>::is_rvalue_ref_qualified;

/*For c-style variadic function type*/
template<typename Ret, typename... Args>
constexpr bool callable_traits<Ret(Args..., ...)>::is_c_style_variadic_function;

//only-cv qualified
template<typename Ret, typename... Args>
constexpr bool callable_traits<Ret(Args..., ...) const>::is_c_style_variadic_function;
template<typename Ret, typename... Args>
constexpr bool callable_traits<Ret(Args..., ...) volatile>::is_c_style_variadic_function;
template<typename Ret, typename... Args>
constexpr bool callable_traits<Ret(Args..., ...) const volatile>::is_c_style_variadic_function;

//lvalue-ref qualified
template<typename Ret, typename... Args>
constexpr bool callable_traits<Ret(Args..., ...) &>::is_c_style_variadic_function;
template<typename Ret, typename... Args>
constexpr bool callable_traits<Ret(Args..., ...) const &>::is_c_style_variadic_function;
template<typename Ret, typename... Args>
constexpr bool callable_traits<Ret(Args..., ...) volatile &>::is_c_style_variadic_function;
template<typename Ret, typename... Args>
constexpr bool callable_traits<Ret(Args..., ...) const volatile &>::is_c_style_variadic_function;

//rvalue-ref qualified
template<typename Ret, typename... Args>
constexpr bool callable_traits<Ret(Args..., ...) &&>::is_c_style_variadic_function;
template<typename Ret, typename... Args>
constexpr bool callable_traits<Ret(Args..., ...) const &&>::is_c_style_variadic_function;
template<typename Ret, typename... Args>
constexpr bool callable_traits<Ret(Args..., ...) volatile &&>::is_c_style_variadic_function;
template<typename Ret, typename... Args>
constexpr bool callable_traits<Ret(Args..., ...) const volatile &&>::is_c_style_variadic_function;

/*For pointer to function type*/
template<typename Ret, typename... Args>
constexpr bool callable_traits<Ret(*)(Args...)>::is_pointer_to_function;

template<typename Ret, typename... Args>
constexpr bool callable_traits<Ret(*)(Args..., ...)>::is_c_style_variadic_function;

/*For pointer to member function type*/
template<typename T, typename Ret, typename... Args>
constexpr bool callable_traits<Ret(T::*)(Args...)>::is_pointer_to_member_function;

//only-cv qualified
template<typename T, typename Ret, typename... Args>
constexpr bool callable_traits<Ret(T::*)(Args...) const>::is_pointer_to_member_function;
template<typename T, typename Ret, typename... Args>
constexpr bool callable_traits<Ret(T::*)(Args...) volatile>::is_pointer_to_member_function;
template<typename T, typename Ret, typename... Args>
constexpr bool callable_traits<Ret(T::*)(Args...) const volatile>::is_pointer_to_member_function;

//lvalue-ref qualified
template<typename T, typename Ret, typename... Args>
constexpr bool callable_traits<Ret(T::*)(Args...) &>::is_pointer_to_member_function;
template<typename T, typename Ret, typename... Args>
constexpr bool callable_traits<Ret(T::*)(Args...) const &>::is_pointer_to_member_function;
template<typename T, typename Ret, typename... Args>
constexpr bool callable_traits<Ret(T::*)(Args...) volatile &>::is_pointer_to_member_function;
template<typename T, typename Ret, typename... Args>
constexpr bool callable_traits<Ret(T::*)(Args...) const volatile &>::is_pointer_to_member_function;

//rvalue-ref qualified
template<typename T, typename Ret, typename... Args>
constexpr bool callable_traits<Ret(T::*)(Args...) &&>::is_pointer_to_member_function;
template<typename T, typename Ret, typename... Args>
constexpr bool callable_traits<Ret(T::*)(Args...) const &&>::is_pointer_to_member_function;
template<typename T, typename Ret, typename... Args>
constexpr bool callable_traits<Ret(T::*)(Args...) volatile &&>::is_pointer_to_member_function;
template<typename T, typename Ret, typename... Args>
constexpr bool callable_traits<Ret(T::*)(Args...) const volatile &&>::is_pointer_to_member_function;

/*For c-style variadic pointer to member function type*/
template<typename T, typename Ret, typename... Args>
constexpr bool callable_traits<Ret(T::*)(Args..., ...)>::is_c_style_variadic_function;

//only-cv qualified
template<typename T, typename Ret, typename... Args>
constexpr bool callable_traits<Ret(T::*)(Args..., ...) const>::is_c_style_variadic_function;
template<typename T, typename Ret, typename... Args>
constexpr bool callable_traits<Ret(T::*)(Args..., ...) volatile>::is_c_style_variadic_function;
template<typename T, typename Ret, typename... Args>
constexpr bool callable_traits<Ret(T::*)(Args..., ...) const volatile>::is_c_style_variadic_function;

//lvalue-ref qualified
template<typename T, typename Ret, typename... Args>
constexpr bool callable_traits<Ret(T::*)(Args..., ...) &>::is_c_style_variadic_function;
template<typename T, typename Ret, typename... Args>
constexpr bool callable_traits<Ret(T::*)(Args..., ...) const &>::is_c_style_variadic_function;
template<typename T, typename Ret, typename... Args>
constexpr bool callable_traits<Ret(T::*)(Args..., ...) volatile &>::is_c_style_variadic_function;
template<typename T, typename Ret, typename... Args>
constexpr bool callable_traits<Ret(T::*)(Args..., ...) const volatile &>::is_c_style_variadic_function;

//rvalue-ref qualified
template<typename T, typename Ret, typename... Args>
constexpr bool callable_traits<Ret(T::*)(Args..., ...) &&>::is_c_style_variadic_function;
template<typename T, typename Ret, typename... Args>
constexpr bool callable_traits<Ret(T::*)(Args..., ...) const &&>::is_c_style_variadic_function;
template<typename T, typename Ret, typename... Args>
constexpr bool callable_traits<Ret(T::*)(Args..., ...) volatile &&>::is_c_style_variadic_function;
template<typename T, typename Ret, typename... Args>
constexpr bool callable_traits<Ret(T::*)(Args..., ...) const volatile &&>::is_c_style_variadic_function;

CEPO_NAMESPACE_END

#endif //CEPO_CALLABLE_TRAITS_HPP
