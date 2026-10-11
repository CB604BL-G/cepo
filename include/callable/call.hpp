//Copyright (c) 2026 CB604BL
#ifndef CEPO_CALLABLE_CALL_HPP
#define CEPO_CALLABLE_CALL_HPP

#include "callable/call_category.hpp"
#include "callable/is_callable.hpp"
#include "configs/namespace_macro.h"
#include "utility/always_false.hpp"
#include <utility>

CEPO_NAMESPACE_START;

//=== if return void ===//
template<typename F, typename... Args>
inline auto call(F&& f, Args&&... args)
	noexcept(is_nothrow_callable<F, Args...>::value)
	-> typename callable::if_return_void_and_basic_call<F, Args...>::type
{
	std::forward<F>(f)(std::forward<Args>(args)...);
}

template<typename F, typename Obj, typename... Args>
inline auto call(F&& f, Obj&& obj, Args&&... args)
	noexcept(is_nothrow_callable<F, Obj, Args...>::value)
	-> typename callable::if_return_void_and_pointer_to_member_call_by_object<F, Obj, Args...>::type
{
	(std::forward<Obj>(obj).*std::forward<F>(f))(std::forward<Args>(args)...);
}

template<typename F, typename Ptr, typename... Args>
inline auto call(F&& f, Ptr&& ptr, Args&&... args)
	noexcept(is_nothrow_callable<F, Ptr, Args...>::value)
	-> typename callable::if_return_void_and_pointer_to_member_call_by_pointer<F, Ptr, Args...>::type
{
	(std::forward<Ptr>(ptr)->*std::forward<F>(f))(std::forward<Args>(args)...);
}

//=== if not return void ===//
template<typename F, typename... Args>
inline auto call(F&& f, Args&&... args)
	noexcept(is_nothrow_callable<F, Args...>::value)
	-> typename callable::if_not_return_void_and_basic_call<F, Args...>::type
{
	return std::forward<F>(f)(std::forward<Args>(args)...);
}

template<typename F, typename Obj, typename... Args>
inline auto call(F&& f, Obj&& obj, Args&&... args)
	noexcept(is_nothrow_callable<F, Obj, Args...>::value)
	-> typename callable::if_not_return_void_and_pointer_to_member_call_by_object<F, Obj, Args...>::type
{
	return (std::forward<Obj>(obj).*std::forward<F>(f))(std::forward<Args>(args)...);
}

template<typename F, typename Ptr, typename... Args>
inline auto call(F&& f, Ptr&& ptr, Args&&... args)
	noexcept(is_nothrow_callable<F, Ptr, Args...>::value)
	-> typename callable::if_not_return_void_and_pointer_to_member_call_by_pointer<F, Ptr, Args...>::type
{
	return (std::forward<Ptr>(ptr)->*std::forward<F>(f))(std::forward<Args>(args)...);
}

//=== invalid call ===//
template<typename F, typename... Args>
inline auto call(F&&, Args&&...) noexcept
	-> typename callable::if_invalid_call<F, Args...>::type
{
	static_assert(always_false<F, Args...>::value, "cepo::call: Invalid call");
}

CEPO_NAMESPACE_END;

#endif //CEPO_CALLABLE_CALL_HPP
