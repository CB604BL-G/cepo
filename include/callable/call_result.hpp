//Copyright (c) 2026 CB604BL
#ifndef CEPO_CALLABLE_CALL_RESULT_HPP
#define CEPO_CALLABLE_CALL_RESULT_HPP

#include "callable/call_category_macros.hpp"
#include "callable/is_callable.hpp"
#include "configs/namespace_macro.h"
#include "utility/priority_tag.hpp"

CEPO_NAMESPACE_START;

#define BASIC_CALL(F, Args) \
	CEPO_CALLABLE_CALL_CATEGORY_EXPR_BASIC_CALL(F, Args)

#define POINTER_TO_MEMBER_CALL_BY_OBJECT(F, Obj, Args) \
	CEPO_CALLABLE_CALL_CATEGORY_EXPR_POINTER_TO_MEMBER_CALL_BY_OBJECT(F, Obj, Args)

#define POINTER_TO_MEMBER_CALL_BY_POINTER(F, Ptr, Args) \
	CEPO_CALLABLE_CALL_CATEGORY_EXPR_POINTER_TO_MEMBER_CALL_BY_POINTER(F, Ptr, Args)

namespace callable
{
	//to avoid hard errors
	template<bool callable, typename F, typename... Args>
	struct call_result_impl {};

	template<typename F, typename... Args>
	struct call_result_impl<true, F, Args...>
	{
	private:
		template<typename F_T, typename... Args_T>
		static auto test(priority_tag<2>) ->
			decltype(BASIC_CALL(F_T, Args_T));

		template<typename F_O, typename Obj, typename... Args_O>
		static auto test(priority_tag<1>) ->
			decltype(POINTER_TO_MEMBER_CALL_BY_OBJECT(F_O, Obj, Args_O));

		template<typename F_P, typename Ptr, typename... Args_P>
		static auto test(priority_tag<0>) ->
			decltype(POINTER_TO_MEMBER_CALL_BY_POINTER(F_P, Ptr, Args_P));

	public:
		using type = decltype(test<F, Args...>(priority_tag<2>{}));
	};
}

template<typename F, typename... Args>
struct call_result
	: callable::call_result_impl<is_callable<F, Args...>::value, F, Args...>
{};

#undef BASIC_CALL
#undef POINTER_TO_MEMBER_CALL_BY_OBJECT
#undef POINTER_TO_MEMBER_CALL_BY_POINTER

CEPO_NAMESPACE_END;

#endif //CEPO_CALLABLE_CALL_RESULT_HPP
