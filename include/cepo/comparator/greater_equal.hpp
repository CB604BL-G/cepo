//Copyright (c) 2026 CB604BL
//CEPO - C++ Eleven Patches Open-source
#ifndef CEPO_COMPARATOR_GREATER_EQUAL_HPP
#define CEPO_COMPARATOR_GREATER_EQUAL_HPP

#include "cepo/configs/namespace_macro.hpp"
#include "cepo/type_traits/is_bool_testable.hpp"
#include "cepo/type_traits/is_nothrow_bool_testable.hpp"
#include <type_traits>

//CEPO Standard Comparators
CEPO_NAMESPACE_START_OF_MODULE(comparator);

struct greater_equal
{
	template<typename T, typename U>
	constexpr auto operator()(T&& l, U&& r) const
		noexcept(
			is_nothrow_bool_testable<decltype(l >= r)>::value and
			noexcept(l >= r))
		-> typename std::enable_if<
			is_bool_testable<decltype(l >= r)>::value,
			bool
		>::type
	{
		return l >= r;
	}
};

CEPO_NAMESPACE_END_OF_MODULE; //comparator

#endif //CEPO_COMPARATOR_GREATER_EQUAL_HPP
