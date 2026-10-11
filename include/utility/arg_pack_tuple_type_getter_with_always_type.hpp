//Copyright (c) 2026 CB604BL
#ifndef CEPO_UTILITY_ARG_PACK_TUPLE_TYPE_GETTER_WITH_ALWAYS_TYPE_HPP
#define CEPO_UTILITY_ARG_PACK_TUPLE_TYPE_GETTER_WITH_ALWAYS_TYPE_HPP

#include "configs/namespace_macro.h"
#include <cstddef>

CEPO_NAMESPACE_START;

namespace utility
{
	template<
		typename arg_pack_tuple_type,
		std::size_t index,
		bool in_range = index < arg_pack_tuple_type::size>
	struct arg_pack_tuple_type_getter_with_always_type
	{
	private:
		struct always_type{};

	public:
		using type = always_type;
	};

	template<typename arg_pack_tuple_type, std::size_t index>
	struct arg_pack_tuple_type_getter_with_always_type<arg_pack_tuple_type, index, true>
	{
		using type = typename arg_pack_tuple_type::template at<index>::type;
	};
}

CEPO_NAMESPACE_END;

#endif //CEPO_UTILITY_ARG_PACK_TUPLE_TYPE_GETTER_WITH_ALWAYS_TYPE_HPP
