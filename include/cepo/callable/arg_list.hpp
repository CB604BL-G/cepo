//Copyright (c) 2026 CB604BL
//CEPO - C++ Eleven Patches Open-source
#ifndef CEPO_CALLABLE_ARG_LIST_HPP
#define CEPO_CALLABLE_ARG_LIST_HPP

#include "cepo/configs/namespace_macro.h"
#include <cstddef>

CEPO_NAMESPACE_START;

template<typename... Args>
class arg_list
{
private:
	template<std::size_t leftover, typename Head, typename... Tail>
	struct at_impl
	{
		using type = typename at_impl<leftover - 1, Tail...>::type;
	};

	template<typename Head, typename... Tail>
	struct at_impl<0, Head, Tail...>
	{
		using type = Head;
	};

	//Used to prevent excessive instantiation.
	template<std::size_t index, bool valid = (index < sizeof...(Args))>
	struct at_helper
	{
		using type = void;
	};

	template<std::size_t index>
	struct at_helper<index, true>
	{
		using type = typename at_impl<index, Args...>::type;
	};
	
public:
	static constexpr std::size_t size = sizeof...(Args);

	template<std::size_t index>
	struct at
	{
		//The at interface drops probing capability.
		//TODO: Add a try_at interface that supports probing capability.
		static_assert(
			index < sizeof...(Args),
			"cepo::arg_list::at: Out of range");

		using type = typename at_helper<index>::type;
	};
};

#if not defined(__cpp_inline_variables)
template<typename... Args>
constexpr std::size_t arg_list<Args...>::size;
#endif //not defined(__cpp_inline_variables)

CEPO_NAMESPACE_END;

#endif //CEPO_CALLABLE_ARG_LIST_HPP
