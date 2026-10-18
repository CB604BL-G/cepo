//Copyright (c) 2026 CB604BL
#ifndef CEPO_RESULT_HPP
#define CEPO_RESULT_HPP

#include "cepo/configs/namespace_macro.h"
#include "cepo/result/slot_base.hpp"
#include <algorithm>
#include <exception>
#include <new>			// IWYU pragma: keep
#include <type_traits>

CEPO_NAMESPACE_START;

template<typename T, typename E>
class result
{
private:
	static_assert(
		std::is_nothrow_destructible<T>::value and
		std::is_nothrow_destructible<E>::value,
		""
	);
	
	union area_type
	{
		T value;
		E error;
		area_type() noexcept {}
		~area_type() noexcept {}
	};

	enum class state_type : unsigned char
	{
		has_value,
		has_error,
		empty
	};

	area_type area;
	std::exception_ptr exception_ptr;
	state_type state;

	void destroy_self() noexcept
	{
		if (state == state_type::has_value)
			area.value.~T();
		else if (state == state_type::has_error)
			area.error.~E();

		state = state_type::empty;
		exception_ptr = nullptr;
	}

public:
	result(result&& other) noexcept:
		area(),
		exception_ptr(nullptr),
		state(state_type::empty)
	{
		if (other.is_empty())
		{
			exception_ptr = other.exception_ptr;
			other.exception_ptr = nullptr;
			return;
		}

		try
		{
			if (other.is_ok())
			{
				new (&area.value) T(std::move(other.area.value));
				state = state_type::has_value;
			}
			else
			{
				new (&area.error) E(std::move(other.area.error));
				state = state_type::has_error;
			}
		}
		catch (...)
		{
			exception_ptr = std::current_exception();
		}

		other.destroy_self();
	}

	result(result_value_type<T>&& value) noexcept:
		area(),
		exception_ptr(nullptr),
		state(state_type::empty)
	{
		if (value.is_empty)
		{
			exception_ptr = value.exception_ptr;
			value.exception_ptr = nullptr;
			return;
		}

		try
		{
			new (&area.value) T(std::move(value.area.slot));
			state = state_type::has_value;
		}
		catch (...)
		{
			exception_ptr = std::current_exception();
		}

		value.destroy_self();
	}

	result(result_error_type<E>&& error) noexcept:
		area(),
		exception_ptr(nullptr),
		state(state_type::empty)
	{
		if (error.is_empty)
		{
			exception_ptr = error.exception_ptr;
			error.exception_ptr = nullptr;
			return;
		}

		try
		{
			new (&area.error) E(std::move(error.area.slot));
			state = state_type::has_error;
		}
		catch (...)
		{
			exception_ptr = std::current_exception();
		}

		error.destroy_self();
	}

	result& operator=(result&& other) noexcept
	{
		if (this == &other) return *this;

		destroy_self();

		if (other.is_empty())
		{
			exception_ptr = other.exception_ptr;
			other.exception_ptr = nullptr;
			return *this;
		}

		try
		{
			if (other.is_ok())
			{
				new (&area.value) T(std::move(other.area.value));
				state = state_type::has_value;
			}
			else
			{
				new (&area.error) E(std::move(other.area.error));
				state = state_type::has_error;
			}
		}
		catch (...)
		{
			exception_ptr = std::current_exception();
		}

		other.destroy_self();

		return *this;
	}

	~result() noexcept
	{
		destroy_self();
	}

	bool is_ok() const noexcept { return state == state_type::has_value; }
	bool is_error() const noexcept { return state == state_type::has_error; }
	bool is_empty() const noexcept { return state == state_type::empty; }
	bool has_exception() const noexcept { return bool(exception_ptr); }
	
	result() = delete;
	result(const result&) = delete;
	result& operator=(const result&) = delete;
};

template<typename E>
class result<void, E>
{
public:

};

template<typename E>
class result<const void, E>
	: result<void, E>
{
public:
	using result<void, E>::result;
	using result<void, E>::operator=;
};


template<typename E>
class result<volatile void, E>
	: result<void, E>
{
public:
	using result<void, E>::result;
	using result<void, E>::operator=;
};


template<typename E>
class result<const volatile void, E>
	: result<void, E>
{
public:
	using result<void, E>::result;
	using result<void, E>::operator=;
};


CEPO_NAMESPACE_END;

#endif //CEPO_RESULT_HPP
