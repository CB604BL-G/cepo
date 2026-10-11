//Copyright (c) 2026 CB604BL
#ifndef CEPO_UTILITY_ARG_PACK_TUPLE_HPP
#define CEPO_UTILITY_ARG_PACK_TUPLE_HPP

#include "configs/namespace_macro.h"
#include <cstddef>

CEPO_NAMESPACE_START;

template<typename... Args>
class arg_pack_tuple
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
		static_assert(
			sizeof...(Args) != 0,
			"cepo::arg_pack_tuple::at: There is no parameter to get");

		static_assert(
			index < sizeof...(Args),
			"cepo::arg_pack_tuple::at: Out of range");

		using type = typename at_helper<index>::type;
	};
};

template<typename... Args>
constexpr std::size_t arg_pack_tuple<Args...>::size;

CEPO_NAMESPACE_END;

#endif //CEPO_UTILITY_ARG_PACK_TUPLE_HPP
