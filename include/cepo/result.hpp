//Copyright (c) 2026 CB604BL
#ifndef CEPO_RESULT_HPP
#define CEPO_RESULT_HPP

#include "cepo/callable/call.hpp"
#include "cepo/callable/call_result.hpp"
#include "cepo/callable/is_callable.hpp"
#include "cepo/configs/namespace_macro.h"
#include "cepo/macros/nodiscard.hpp"
#include "cepo/result/base_void_helper.hpp"
#include "cepo/result/result_base.hpp"
#include "cepo/result/when_error.hpp"
#include <type_traits>
#include <utility>

CEPO_NAMESPACE_START;

template<typename T, typename E>
class result
	: private impl::result_base<T, E>
{
private:
	static_assert(std::is_object<T>::value && std::is_object<E>::value,
		"cepo::result: T and E must be object types (no references/void/function)");

	static_assert(std::is_move_constructible<T>::value &&
		std::is_move_constructible<E>::value,
		"cepo::result: T and E must be move-constructible");

	static_assert(std::is_nothrow_destructible<T>::value &&
		std::is_nothrow_destructible<E>::value,
		"cepo::result: T and E must have nothrow destructors");

	using base_type = impl::result_base<T, E>;

public:
	using impl::result_base<T, E>::result_base;

	using base_type::is_ok;
	using base_type::is_error;
	using base_type::is_empty;
	using base_type::has_exception;

	using base_type::get_exception;
	using base_type::unwrap_value;
	using base_type::unwrap_error;

	template<typename F>
	CEPO_NODISCARD
	auto ok(F&& callback)
	noexcept(is_nothrow_callable<F, T&>::value)
		-> typename std::enable_if<
			is_callable<F, T&>::value and
			std::is_void<
				typename call_result<F, T&>::type>::value,
			result_when_error<E>
		>::type
	{
		if (is_ok())
			call(
				std::forward<F>(callback),
				this->get_value_unchecked());
		return result_when_error<E>{
			is_error() ?
			&this->get_error_unchecked() :
			nullptr
		};
	}

	template<typename F>
	CEPO_NODISCARD
	auto ok(F&& callback) const
	noexcept(is_nothrow_callable<F, const T&>::value)
		-> typename std::enable_if<
			is_callable<F, const T&>::value and
			std::is_void<
				typename call_result<F, const T&>::type>::value,
			result_when_error<E>
		>::type
	{
		if (is_ok())
			call(
				std::forward<F>(callback),
				this->get_value_unchecked());
		return result_when_error<E>{
			is_error() ?
			&this->get_error_unchecked() :
			nullptr
		};
	}
};

template<typename E>
class result<void, E>
	: private impl::result_base_void_helper<void, E>
{
private:
	static_assert(std::is_object<E>::value,
		"cepo::result: E must be object types (no references/void/function)");

	static_assert(
		std::is_move_constructible<E>::value,
		"cepo::result: E must be move-constructible");

	static_assert(
		std::is_nothrow_destructible<E>::value,
		"cepo::result: E must have nothrow destructor");

	using base_type = impl::result_base_void_helper<void, E>;
public:
	using impl::result_base_void_helper<void, E>::result_base_void_helper;

	using base_type::is_ok;
	using base_type::is_error;
	using base_type::is_empty;
	using base_type::has_exception;

	using base_type::get_exception;
	using base_type::unwrap_error;
	using base_type::ok;
};

template<typename E>
class result<const void, E>
	: private impl::result_base_void_helper<const void, E>
{
private:
	static_assert(std::is_object<E>::value,
		"cepo::result: E must be object types (no references/void/function)");

	static_assert(
		std::is_move_constructible<E>::value,
		"cepo::result: E must be move-constructible");

	static_assert(
		std::is_nothrow_destructible<E>::value,
		"cepo::result: E must have nothrow destructor");

	using base_type = impl::result_base_void_helper<const void, E>;

public:
	using impl::result_base_void_helper<const void, E>::result_base_void_helper;

	using base_type::is_ok;
	using base_type::is_error;
	using base_type::is_empty;
	using base_type::has_exception;

	using base_type::get_exception;
	using base_type::unwrap_error;
	using base_type::ok;
};

template<typename E>
class result<volatile void, E>
	: private impl::result_base_void_helper<volatile void, E>
{
private:
	static_assert(std::is_object<E>::value,
		"cepo::result: E must be object types (no references/void/function)");

	static_assert(
		std::is_move_constructible<E>::value,
		"cepo::result: E must be move-constructible");

	static_assert(
		std::is_nothrow_destructible<E>::value,
		"cepo::result: E must have nothrow destructor");

	using base_type = impl::result_base_void_helper<volatile void, E>;

public:
	using impl::result_base_void_helper<volatile void, E>::result_base_void_helper;

	using base_type::is_ok;
	using base_type::is_error;
	using base_type::is_empty;
	using base_type::has_exception;

	using base_type::get_exception;
	using base_type::unwrap_error;
	using base_type::ok;
};

template<typename E>
class result<const volatile void, E>
	: private impl::result_base_void_helper<const volatile void, E>
{
private:
	static_assert(std::is_object<E>::value,
		"cepo::result: E must be object types (no references/void/function)");

	static_assert(
		std::is_move_constructible<E>::value,
		"cepo::result: E must be move-constructible");

	static_assert(
		std::is_nothrow_destructible<E>::value,
		"cepo::result: E must have nothrow destructor");

	using base_type = impl::result_base_void_helper<const volatile void, E>;

public:
	using impl::result_base_void_helper<const volatile void, E>::result_base_void_helper;

	using base_type::is_ok;
	using base_type::is_error;
	using base_type::is_empty;
	using base_type::has_exception;

	using base_type::get_exception;
	using base_type::unwrap_error;
	using base_type::ok;
};

CEPO_NAMESPACE_END;

#endif //CEPO_RESULT_HPP
