//Copyright (c) 2026 CB604BL
//CEPO - C++ Eleven Patches Open-source
#ifndef CEPO_RESULT_SLOTS_HPP
#define CEPO_RESULT_SLOTS_HPP

#include "cepo/configs/namespace_macro.hpp"
#include "cepo/result/slot_base.hpp"
#include "cepo/result/slot_tags.hpp"
#include <type_traits>

CEPO_NAMESPACE_START;

template<typename T>
class result_value_slot
	: private impl::result_slot_base<T, impl::result_slot_value_tag>
{
private:
	static_assert(std::is_object<T>::value,
		"cepo::result_value_slot: T must be an object type (no references/void/function)");

	static_assert(
		std::is_move_constructible<T>::value,
		"cepo::result_value_slot: T must be move-constructible");

	static_assert(
		std::is_nothrow_destructible<T>::value,
		"cepo::result_value_slot: T must have nothrow destructor");
	
	using tag_type = impl::result_slot_value_tag;
	using base_type = impl::result_slot_base<T, tag_type>;

public:
	using impl::result_slot_base<T, tag_type>::result_slot_base;

	using base_type::operator*;
	using base_type::is_exceptional;
	using base_type::get_exception;
};

template<>
class result_value_slot<void>
	: private impl::result_slot_base<void, impl::result_slot_value_tag>
{
private:
	using tag_type = impl::result_slot_value_tag;
	using base_type = impl::result_slot_base<void, tag_type>;

public:
	using impl::result_slot_base<void, tag_type>::result_slot_base;

	using base_type::is_exceptional;
	using base_type::get_exception;
};

template<>
class result_value_slot<const void>
	: private impl::result_slot_base<const void, impl::result_slot_value_tag>
{
private:
	using tag_type = impl::result_slot_value_tag;
	using base_type = impl::result_slot_base<const void, tag_type>;

public:
	using impl::result_slot_base<const void, tag_type>::result_slot_base;

	using base_type::is_exceptional;
	using base_type::get_exception;
};

template<>
class result_value_slot<volatile void>
	: private impl::result_slot_base<volatile void, impl::result_slot_value_tag>
{
private:
	using tag_type = impl::result_slot_value_tag;
	using base_type = impl::result_slot_base<volatile void, tag_type>;

public:
	using impl::result_slot_base<volatile void, tag_type>::result_slot_base;

	using base_type::is_exceptional;
	using base_type::get_exception;
};

template<>
class result_value_slot<const volatile void>
	: private impl::result_slot_base<const volatile void, impl::result_slot_value_tag>
{
private:
	using tag_type = impl::result_slot_value_tag;
	using base_type = impl::result_slot_base<const volatile void, tag_type>;

public:
	using impl::result_slot_base<const volatile void, tag_type>::result_slot_base;

	using base_type::is_exceptional;
	using base_type::get_exception;
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
	
	using tag_type = impl::result_slot_error_tag;
	using base_type = impl::result_slot_base<E, tag_type>;

public:
	using impl::result_slot_base<E, tag_type>::result_slot_base;

	using base_type::operator*;
	using base_type::is_exceptional;
	using base_type::get_exception;
};

CEPO_NAMESPACE_END;

#endif //CEPO_RESULT_SLOTS_HPP
