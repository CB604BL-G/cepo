//Copyright (c) 2026 CB604BL
//CEPO - C++ Eleven Patches Open-source
#ifndef CEPO_RESULT_SLOTS_HPP
#define CEPO_RESULT_SLOTS_HPP

#include "cepo/configs/namespace_macro.h"
#include "cepo/result/slot_base.hpp"
#include "cepo/result/slot_tags.hpp"
#include "cepo/type_traits/conjunction.hpp"
#include "cepo/type_traits/disjunction.hpp"
#include "cepo/type_traits/negation.hpp"
#include <type_traits>

CEPO_NAMESPACE_START;

template<typename T>
class result_value_slot
	: private impl::result_slot_base<T, impl::result_slot_value_tag>
{
private:
	static_assert(std::is_object<T>::value or std::is_void<T>::value,
		"cepo::result_value_slot: T must be object types or void type (no references/function)");

	static_assert(
		disjunction<
			conjunction<
				negation<std::is_void<T>>, std::is_move_constructible<T>>,
			std::is_void<T>
		>::value,
		"cepo::result_value_slot: T must be move-constructible if T is not void type");

	static_assert(
		disjunction<
			conjunction<
				negation<std::is_void<T>>, std::is_nothrow_destructible<T>>,
			std::is_void<T>
		>::value,
		"cepo::result_value_slot: T must have nothrow destructor if T is not void type");
	
	template<typename Tp, typename Ep>
	friend class ::cepo::impl::result_base;

public:
	using impl::result_slot_base<T, impl::result_slot_value_tag>::result_slot_base;
};

template<typename E>
class result_error_slot
	: private impl::result_slot_base<E, impl::result_slot_error_tag>
{
private:
	static_assert(std::is_object<E>::value,
		"cepo::result_error_slot: E must be object types (no references/void/function)");

	static_assert(
		std::is_move_constructible<E>::value,
		"cepo::result_error_slot: E must be move-constructible");

	static_assert(
		std::is_nothrow_destructible<E>::value,
		"cepo::result_error_slot: E must have nothrow destructor");
	
	template<typename Tp, typename Ep>
	friend class ::cepo::impl::result_base;

public:
	using impl::result_slot_base<E, impl::result_slot_error_tag>::result_slot_base;
};

CEPO_NAMESPACE_END;

#endif //CEPO_RESULT_SLOTS_HPP
