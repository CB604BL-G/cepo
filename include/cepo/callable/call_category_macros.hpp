//Copyright (c) 2026 CB604BL
#ifndef CEPO_CALLABLE_CALL_CATEGORY_MACROS_HPP
#define CEPO_CALLABLE_CALL_CATEGORY_MACROS_HPP

#include <utility>		// IWYU pragma: keep

//These are macros that store expressions
#define CEPO_CALLABLE_CALL_CATEGORY_EXPR_BASIC_CALL(F, Args) \
	(std::declval<F>()(std::declval<Args>()...))

#define CEPO_CALLABLE_CALL_CATEGORY_EXPR_POINTER_TO_MEMBER_CALL_BY_OBJECT(F, Obj, Args) \
	((std::declval<Obj>().*std::declval<F>())(std::declval<Args>()...))

#define CEPO_CALLABLE_CALL_CATEGORY_EXPR_POINTER_TO_MEMBER_CALL_BY_POINTER(F, Ptr, Args) \
	((std::declval<Ptr>()->*std::declval<F>())(std::declval<Args>()...))

#endif //CEPO_CALLABLE_CALL_CATEGORY_MACROS_HPP
