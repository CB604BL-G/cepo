//Copyright (c) 2026 CB604BL
#ifndef CEPO_CALLABLE_CALL_CATEGORY_HPP
#define CEPO_CALLABLE_CALL_CATEGORY_HPP

#include "cepo/callable/call_result.hpp"
#include "cepo/configs/namespace_macro.h"
#include "cepo/utility/priority_tag.hpp"
#include <type_traits>

CEPO_NAMESPACE_START;

namespace impl
{
	struct basic_call_category{};
	struct pointer_to_member_call_by_object_category{};
	struct pointer_to_member_call_by_pointer_category{};
	//Just to avoid hard errors
	struct invalid_call_category{};

	//Use is_callable to check it first before using
	template<typename F, typename... Args>
	struct get_call_category
	{
	private:
		template<typename F_T, typename... Args_T>
		static auto test(priority_tag<2>) ->
			decltype(
				CEPO_CALLABLE_CALL_CATEGORY_EXPR_BASIC_CALL(F_T, Args_T),
				basic_call_category{});

		template<typename F_O, typename Obj, typename... Args_O>
		static auto test(priority_tag<1>) ->
			decltype(
				CEPO_CALLABLE_CALL_CATEGORY_EXPR_POINTER_TO_MEMBER_CALL_BY_OBJECT(F_O, Obj, Args_O),
				pointer_to_member_call_by_object_category{});

		template<typename F_P, typename Ptr, typename... Args_P>
		static auto test(priority_tag<0>) ->
			decltype(
				CEPO_CALLABLE_CALL_CATEGORY_EXPR_POINTER_TO_MEMBER_CALL_BY_POINTER(F_P, Ptr, Args_P),
				pointer_to_member_call_by_pointer_category{});

		template<typename...>
		static invalid_call_category test(...);

	public:
		using type = decltype(test<F, Args...>(priority_tag<2>{}));
	};

	//To avoid enable_if instantiation of failed
	template<typename Category, typename F, typename... Args>
	struct call_result_with_always_type
	{
		using type = typename call_result<F, Args...>::type;
	};

	template<typename F, typename... Args>
	struct call_result_with_always_type<invalid_call_category, F, Args...>
	{
		using type = void;
	};

	template<typename F, typename... Args>
	using call_result_or_void = typename call_result_with_always_type<
		typename get_call_category<F, Args...>::type, F, Args...>::type;

	template<typename F, typename... Args>
	using call_category_of = typename get_call_category<F, Args...>::type;

	/*The following are mutually exclusive category condition*/
	//if return void
	template<typename F, typename... Args>
	struct if_return_void_and_basic_call
		: std::enable_if<
			std::is_void<call_result_or_void<F, Args...>>::value and
			std::is_same<basic_call_category, call_category_of<F, Args...>>::value,
			void>
	{};

	template<typename F, typename... Args>
	struct if_return_void_and_pointer_to_member_call_by_object
		: std::enable_if<
			std::is_void<call_result_or_void<F, Args...>>::value and
			std::is_same<pointer_to_member_call_by_object_category, call_category_of<F, Args...>>::value,
			void>
	{};

	template<typename F, typename... Args>
	struct if_return_void_and_pointer_to_member_call_by_pointer
		: std::enable_if<
			std::is_void<call_result_or_void<F, Args...>>::value and
			std::is_same<pointer_to_member_call_by_pointer_category, call_category_of<F, Args...>>::value,
			void>
	{};

	//if not return void
	template<typename F, typename... Args>
	struct if_not_return_void_and_basic_call
		: std::enable_if<
			not std::is_void<call_result_or_void<F, Args...>>::value and
			std::is_same<basic_call_category, call_category_of<F, Args...>>::value,
			call_result_or_void<F, Args...>>
	{};

	template<typename F, typename... Args>
	struct if_not_return_void_and_pointer_to_member_call_by_object
		: std::enable_if<
			not std::is_void<call_result_or_void<F, Args...>>::value and
			std::is_same<pointer_to_member_call_by_object_category, call_category_of<F, Args...>>::value,
			call_result_or_void<F, Args...>>
	{};

	template<typename F, typename... Args>
	struct if_not_return_void_and_pointer_to_member_call_by_pointer
		: std::enable_if<
			not std::is_void<call_result_or_void<F, Args...>>::value and
			std::is_same<pointer_to_member_call_by_pointer_category, call_category_of<F, Args...>>::value,
			call_result_or_void<F, Args...>>
	{};

	//if invalid
	template<typename F, typename... Args>
	struct if_invalid_call
		: std::enable_if<
			std::is_same<invalid_call_category, call_category_of<F, Args...>>::value,
			void>
	{};
}

CEPO_NAMESPACE_END;

#endif //CEPO_CALLABLE_CALL_CATEGORY_HPP
