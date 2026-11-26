//Copyright (c) 2026 CB604BL
//CEPO - C++ Eleven Patches Open-source
#ifndef CEPO_RESULT_HPP
#define CEPO_RESULT_HPP

#include "cepo/callable/call.hpp"
#include "cepo/callable/call_result.hpp"
#include "cepo/callable/is_callable.hpp"
#include "cepo/callable/is_nothrow_callable.hpp"
#include "cepo/comparator/equal_to.hpp"
#include "cepo/comparator/not_equal_to.hpp"
#include "cepo/configs/namespace_macro.hpp"
#include "cepo/macros/nodiscard.hpp"
#include "cepo/result/base_void_helper.hpp"
#include "cepo/result/result_base.hpp"
#include "cepo/result/when_error.hpp"
#include <type_traits>
#include <utility>

CEPO_NAMESPACE_START;

//This is a move-only, copy-deleted class that holds the result
//of an operation, the error that made it fail, or the exception
//that escaped while it was being built. It is in exactly one of
//three states at every moment of its lifetime: ok, error, or
//exceptional. The three share a union, so nothing is allocated,
//and the state tag is what decides which member is alive.

//A result is never constructed from a value directly.
//make_result_value and make_result_error build a slot holding T
//or E, and a result is built from that slot. If the slot failed
//to construct its payload, or if moving the payload out of the
//slot throws, the result is exceptional and the exception that
//was thrown is recorded.

//The exceptional state means that the last operation threw and
//that there is no value and no error to hand out. It is not a
//state one recovers from by waiting: it stays until the object
//is assigned or destroyed, and while it holds only the state
//queries, the exception accessor and the propagation chain are
//meaningful.

//Only unwrap_value and unwrap_error throw. They throw
//bad_result_unwrap, which carries a reason identifying the
//state the call was invalid in; the recorded exception is not
//part of it, use get_exception to obtain the exception_ptr and
//rethrow it yourself. Everything else is noexcept: construction,
//move assignment, and the whole propagation chain, with the
//callbacks being the only user code that can throw through it.

//Moving in the exceptional state shares the exception. No new
//exception is thrown, the source keeps its state, and both
//objects end up referring to the same exception. Moving in any
//other state does not consume the source either: it keeps its
//previous state, and the payload it holds is left valid but
//unspecified.

//Because the slot -> result transfer is always a move, a T or E
//whose move constructor may throw can never produce an ok or
//error result at all: the transfer fails at that step and the
//result is exceptional. Only types that do not throw on move can
//carry a value state.

//The class also owns the propagation chain. ok and ignore_ok
//return a proxy that reports whether an error is present, and
//error and ignore_error on that proxy return one that reports
//whether an exception is present, so the chain reads
//ok -> error -> exceptional. A stage whose state does not apply
//skips its callback, every stage forwards the remaining states
//unchanged, and ignore_ok, ignore_error and ignore_exceptional
//end the chain without running anything. All stages but the last
//are nodiscard, which is what turns an unhandled error or
//exception into a warning instead of something that disappears
//in silence.

//Both proxies are views. They hold pointers into the result they
//came from, they cannot be copied, and their members are rvalue
//qualified, so they are meant to be consumed by the expression
//that produced them. Storing one is possible, but it stays valid
//only while the result it points into is alive.

//Equality compares the state first: results in different states
//are never equal, and the state decides which payload is
//compared, T's operator== for an ok result and E's for an error
//result. Exceptional results compare their exception_ptr, so two
//results that share an exception after a move compare equal.
//operator== requires T and E to be equality comparable and
//operator!= requires them to be inequality comparable; a type
//that provides only one of the two keeps that restriction on the
//result instead of failing somewhere inside the comparison body.
//Neither operator is derived from the other, so a type whose ==
//and != are not negations of each other keeps that asymmetry
//here as well.
template<typename T, typename E>
class result final
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

	template<typename Tp, typename Ep>
	friend auto operator==(const result<Tp, Ep>& lhs, const result<Tp, Ep>& rhs)
		noexcept(noexcept(lhs.compare_with(comparator::equal_to{}, rhs)))
		-> typename std::enable_if<
			std::is_same<Tp, T>::value and
			std::is_same<Ep, E>::value and
			is_callable<comparator::equal_to, const Tp&, const Tp&>::value and
			is_callable<comparator::equal_to, const Ep&, const Ep&>::value,
			bool
		>::type
	{
		return lhs.compare_with(comparator::equal_to{}, rhs);
	}

	template<typename Tp, typename Ep>
	friend auto operator!=(const result<Tp, Ep>& lhs, const result<Tp, Ep>& rhs)
		noexcept(noexcept(lhs.compare_with(comparator::not_equal_to{}, rhs)))
		-> typename std::enable_if<
			std::is_same<Tp, T>::value and
			std::is_same<Ep, E>::value and
			is_callable<comparator::not_equal_to, const Tp&, const Tp&>::value and
			is_callable<comparator::not_equal_to, const Ep&, const Ep&>::value,
			bool
		>::type
	{
		return lhs.compare_with(comparator::not_equal_to{}, rhs);
	}

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
			nullptr,
			is_exceptional() ?
			&this->get_exceptional_unchecked() :
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
			nullptr,
			is_exceptional() ?
			&this->get_exceptional_unchecked() :
			nullptr
		};
	}

	CEPO_NODISCARD
	result_when_error<E> ignore_ok() const noexcept
	{
		return result_when_error<E>{
			is_error() ?
			&this->get_error_unchecked() :
			nullptr,
			is_exceptional() ?
			&this->get_exceptional_unchecked() :
			nullptr
		};
	}
};

template<typename E>
class result<void, E> final
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

	template<typename Vp, typename Ep>
	friend auto operator==(const result<Vp, Ep>& lhs, const result<Vp, Ep>& rhs)
		noexcept(noexcept(lhs.compare_with(comparator::equal_to{}, rhs)))
		-> typename std::enable_if<
			std::is_same<Vp, void>::value and
			std::is_same<Ep, E>::value and
			is_callable<comparator::equal_to, const Ep&, const Ep&>::value,
			bool
		>::type
	{
		return lhs.compare_with(comparator::equal_to{}, rhs);
	}

	template<typename Vp, typename Ep>
	friend auto operator!=(const result<Vp, Ep>& lhs, const result<Vp, Ep>& rhs)
		noexcept(noexcept(lhs.compare_with(comparator::not_equal_to{}, rhs)))
		-> typename std::enable_if<
			std::is_same<Vp, void>::value and
			std::is_same<Ep, E>::value and
			is_callable<comparator::not_equal_to, const Ep&, const Ep&>::value,
			bool
		>::type
	{
		return lhs.compare_with(comparator::not_equal_to{}, rhs);
	}

	using base_type::ok;
	using base_type::ignore_ok;
};

template<typename E>
class result<const void, E> final
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
	
	template<typename Vp, typename Ep>
	friend auto operator==(const result<Vp, Ep>& lhs, const result<Vp, Ep>& rhs)
		noexcept(noexcept(lhs.compare_with(comparator::equal_to{}, rhs)))
		-> typename std::enable_if<
			std::is_same<Vp, const void>::value and
			std::is_same<Ep, E>::value and
			is_callable<comparator::equal_to, const Ep&, const Ep&>::value,
			bool
		>::type
	{
		return lhs.compare_with(comparator::equal_to{}, rhs);
	}
	
	template<typename Vp, typename Ep>
	friend auto operator!=(const result<Vp, Ep>& lhs, const result<Vp, Ep>& rhs)
		noexcept(noexcept(lhs.compare_with(comparator::not_equal_to{}, rhs)))
		-> typename std::enable_if<
			std::is_same<Vp, const void>::value and
			std::is_same<Ep, E>::value and
			is_callable<comparator::not_equal_to, const Ep&, const Ep&>::value,
			bool
		>::type
	{
		return lhs.compare_with(comparator::not_equal_to{}, rhs);
	}

	using base_type::ok;
	using base_type::ignore_ok;
};

template<typename E>
class result<volatile void, E> final
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
	
	template<typename Vp, typename Ep>
	friend auto operator==(const result<Vp, Ep>& lhs, const result<Vp, Ep>& rhs)
		noexcept(noexcept(lhs.compare_with(comparator::equal_to{}, rhs)))
		-> typename std::enable_if<
			std::is_same<Vp, volatile void>::value and
			std::is_same<Ep, E>::value and
			is_callable<comparator::equal_to, const Ep&, const Ep&>::value,
			bool
		>::type
	{
		return lhs.compare_with(comparator::equal_to{}, rhs);
	}

	template<typename Vp, typename Ep>
	friend auto operator!=(const result<Vp, Ep>& lhs, const result<Vp, Ep>& rhs)
		noexcept(noexcept(lhs.compare_with(comparator::not_equal_to{}, rhs)))
		-> typename std::enable_if<
			std::is_same<Vp, volatile void>::value and
			std::is_same<Ep, E>::value and
			is_callable<comparator::not_equal_to, const Ep&, const Ep&>::value,
			bool
		>::type
	{
		return lhs.compare_with(comparator::not_equal_to{}, rhs);
	}

	using base_type::ok;
	using base_type::ignore_ok;
};

template<typename E>
class result<const volatile void, E> final
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

	template<typename Vp, typename Ep>
	friend auto operator==(const result<Vp, Ep>& lhs, const result<Vp, Ep>& rhs)
		noexcept(noexcept(lhs.compare_with(comparator::equal_to{}, rhs)))
		-> typename std::enable_if<
			std::is_same<Vp, const volatile void>::value and
			std::is_same<Ep, E>::value and
			is_callable<comparator::equal_to, const Ep&, const Ep&>::value,
			bool
		>::type
	{
		return lhs.compare_with(comparator::equal_to{}, rhs);
	}
	
	template<typename Vp, typename Ep>
	friend auto operator!=(const result<Vp, Ep>& lhs, const result<Vp, Ep>& rhs)
		noexcept(noexcept(lhs.compare_with(comparator::not_equal_to{}, rhs)))
		-> typename std::enable_if<
			std::is_same<Vp, const volatile void>::value and
			std::is_same<Ep, E>::value and
			is_callable<comparator::not_equal_to, const Ep&, const Ep&>::value,
			bool
		>::type
	{
		return lhs.compare_with(comparator::not_equal_to{}, rhs);
	}

	using base_type::ok;
	using base_type::ignore_ok;
};

CEPO_NAMESPACE_END;

#endif //CEPO_RESULT_HPP
