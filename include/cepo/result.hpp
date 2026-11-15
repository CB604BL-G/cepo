//Copyright (c) 2026 CB604BL
//CEPO - C++ Eleven Patches Open-source
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

//This is a tri-state move-only class. During the lifetime of
//an object it is always in exactly one of three states: ok,
//error, or exceptional. The exceptional state means that the
//stored value or error could not be constructed, and that the
//exception which caused it has been recorded.

//Moreover, the result class is not completely exception-free.
//It only guarantees that no new exceptions will be thrown
//along the error-propagation chain, which is why both its
//constructors and its move assignment operator are noexcept.
//If you call unwrap_value when the state is exceptional or
//error, it will throw an exception rather than silently fail
//and give you an invalid reference. The same applies when
//calling unwrap_error. The exception thrown when unwrap fails
//does not carry the exception that may have occurred during
//construction; use is_exceptional to detect that, and
//get_exception to rethrow it.

//Moving behaves differently depending on the state, which is
//something the caller has to keep in mind. Moving an object in
//the exceptional state propagates the exception: the exception
//is shared with the new object, no exception is thrown, and the
//source keeps its state as well, so both objects afterwards
//refer to the same exception. Moving an object in any other
//state does not consume the source: it keeps its previous
//state, but the value or error it stores is left in a valid
//but unspecified state.

//Note on the slot -> result transfer: it is always a move.
//Because of this, a T (or E) whose move constructor may throw
//can never produce an ok (respectively error) result at all:
//make_result_value / make_result_error already fails at that
//step, and the result ends up exceptional with the exception
//recorded. In other words, "move-constructible" is not enough
//here; only types that do not throw on move can actually carry
//a value state.

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
	using base_type::is_exceptional;

	using base_type::unwrap_value;
	using base_type::unwrap_error;
	using base_type::get_exception;

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
	using base_type::is_exceptional;

	using base_type::unwrap_error;
	using base_type::get_exception;
	
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
	using base_type::is_exceptional;

	using base_type::unwrap_error;
	using base_type::get_exception;
	
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
	using base_type::is_exceptional;

	using base_type::unwrap_error;
	using base_type::get_exception;
	
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
	using base_type::is_exceptional;

	using base_type::unwrap_error;
	using base_type::get_exception;
	
	using base_type::ok;
};

CEPO_NAMESPACE_END;

#endif //CEPO_RESULT_HPP
