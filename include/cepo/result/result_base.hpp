//Copyright (c) 2026 CB604BL
//CEPO - C++ Eleven Patches Open-source
#ifndef CEPO_RESULT_RESULT_BASE_HPP
#define CEPO_RESULT_RESULT_BASE_HPP

#include "cepo/configs/namespace_macro.h"
#include "cepo/result/bad_result_unwrap.hpp"
#include "cepo/result/slots.hpp"
#include <exception>
#include <utility>

CEPO_IMPL_NAMESPACE_START;

template<typename T, typename E>
class result_base
{
private:
	union area_type
	{
		T value;
		E error;
		std::exception_ptr exception_ptr;
		
		area_type() noexcept {}
		~area_type() noexcept {}
	};

	enum class state_type : unsigned char
	{
		ok,
		error,
		exceptional
	};

	area_type area;
	state_type state;

	void destroy_self() noexcept
	{
		if (state == state_type::ok)
			area.value.~T();
		else if (state == state_type::error)
			area.error.~E();
		else if (state == state_type::exceptional)
			area.exception_ptr.~decltype(area.exception_ptr)();
		//Keep current state
	}

protected:
	inline T& get_value_unchecked() noexcept { return area.value; }
	inline const T& get_value_unchecked() const noexcept { return area.value; }

	//const-only
	inline const E& get_error_unchecked() const noexcept { return area.error; }

public:
	result_base(result_base&& other) noexcept:
		area(),
		state(state_type::exceptional)		//Default state
	{
		if (other.is_exceptional())
		{
			//Sharing exception instead of moving
			new (&area.exception_ptr) std::exception_ptr(other.area.exception_ptr);
			return;
		}

		try
		{
			if (other.is_ok())
			{
				new (&area.value) T(std::move(other.area.value));
				state = state_type::ok;
			}
			else
			{
				new (&area.error) E(std::move(other.area.error));
				state = state_type::error;
			}
		}
		catch (...)
		{
			new (&area.exception_ptr) std::exception_ptr(std::current_exception());
		}
	}

	result_base(result_value_slot<T>&& value) noexcept:
		area(),
		state(state_type::exceptional)
	{
		if (not value.is_successful)
		{
			new (&area.exception_ptr) std::exception_ptr(value.area.exception_ptr);
			return;
		}

		try
		{
			new (&area.value) T(std::move(value.area.slot));
			state = state_type::ok;
		}
		catch (...)
		{
			new (&area.exception_ptr) std::exception_ptr(std::current_exception());
		}
	}

	result_base(result_error_slot<E>&& error) noexcept:
		area(),
		state(state_type::exceptional)
	{
		if (not error.is_successful)
		{
			new (&area.exception_ptr) std::exception_ptr(error.area.exception_ptr);
			return;
		}

		try
		{
			new (&area.error) E(std::move(error.area.slot));
			state = state_type::error;
		}
		catch (...)
		{
			new (&area.exception_ptr) std::exception_ptr(std::current_exception());
		}
	}

	result_base& operator=(result_base&& other) noexcept
	{
		if (this == &other) return *this;

		destroy_self();
		state = state_type::exceptional;
		if (other.is_exceptional())
		{
			new (&area.exception_ptr) std::exception_ptr(other.area.exception_ptr);
			return *this;
		}

		try
		{
			if (other.is_ok())
			{
				new (&area.value) T(std::move(other.area.value));
				state = state_type::ok;
			}
			else
			{
				new (&area.error) E(std::move(other.area.error));
				state = state_type::error;
			}
		}
		catch (...)
		{
			new (&area.exception_ptr) std::exception_ptr(std::current_exception());
		}

		return *this;
	}

	~result_base() noexcept
	{
		destroy_self();
	}

	bool is_ok() const noexcept { return state == state_type::ok; }
	bool is_error() const noexcept { return state == state_type::error; }
	bool is_exceptional() const noexcept { return state == state_type::exceptional; }

	//Noexcept, but get in non-exceptional state then rethrow it is UB
	std::exception_ptr get_exception() const noexcept
	{	
		return is_exceptional()
			? area.exception_ptr
			: std::exception_ptr{nullptr};
	}

	T& unwrap_value()
	{
		if (is_error())
			throw bad_result_unwrap(bad_result_unwrap_reason::unwrap_value_in_error_state);
		else if (is_exceptional())
			throw bad_result_unwrap(bad_result_unwrap_reason::unwrap_in_exceptional_state);
		return area.value;
	}
	
	const T& unwrap_value() const
	{
		if (is_error())
			throw bad_result_unwrap(bad_result_unwrap_reason::unwrap_value_in_error_state);
		else if (is_exceptional())
			throw bad_result_unwrap(bad_result_unwrap_reason::unwrap_in_exceptional_state);
		return area.value;
	}

	const E& unwrap_error() const
	{
		if (is_ok())
			throw bad_result_unwrap(bad_result_unwrap_reason::unwrap_error_in_ok_state);
		else if (is_exceptional())
			throw bad_result_unwrap(bad_result_unwrap_reason::unwrap_in_exceptional_state);
		return area.error;
	}
	
	result_base() = delete;
	result_base(const result_base&) = delete;
	result_base& operator=(const result_base&) = delete;
};

template<typename Void, typename E>
class result_base_void
{
private:
	union area_type
	{
		//No need to save void type anymore
		E error;
		std::exception_ptr exception_ptr;
		
		area_type() noexcept {};
		~area_type() noexcept {};
	};

	enum class state_type : unsigned char
	{
		ok,
		error,
		exceptional
	};

	area_type area;
	state_type state;

	void destroy_self() noexcept
	{
		//No need to destruct void
		if (state == state_type::error)
			area.error.~E();
		else if (state == state_type::exceptional)
			area.exception_ptr.~decltype(area.exception_ptr)();
		//Keep current state
	}

protected:
	inline const E& get_error_unchecked() const noexcept { return area.error; }
	
public:
	result_base_void(result_base_void&& other) noexcept:
		area(),
		state(state_type::exceptional)
	{
		if (other.is_exceptional())
		{
			new (&area.exception_ptr) std::exception_ptr(other.area.exception_ptr);
			return;
		}
		else if (other.is_ok())
		{
			state = state_type::ok;
			return;
		}

		try
		{
			new (&area.error) E(std::move(other.area.error));
			state = state_type::error;
		}
		catch (...)
		{
			new (&area.exception_ptr) std::exception_ptr(std::current_exception());
		}
	}

	//There is NO data member in result_value_type when std::is_void<Void>::value is true
	result_base_void(result_value_slot<Void>&&) noexcept:
		area(),
		state(state_type::ok)
	{}

	result_base_void(result_error_slot<E>&& error) noexcept:
		area(),
		state(state_type::exceptional)
	{
		if (not error.is_successful)
		{
			new (&area.exception_ptr) std::exception_ptr(error.area.exception_ptr);
			return;
		}

		try
		{
			new (&area.error) E(std::move(error.area.slot));
			state = state_type::error;
		}
		catch (...)
		{
			new (&area.exception_ptr) std::exception_ptr(std::current_exception());
		}
	}

	result_base_void& operator=(result_base_void&& other) noexcept
	{
		if (this == &other) return *this;

		destroy_self();
		state = state_type::exceptional;
		if (other.is_exceptional())
		{
			new (&area.exception_ptr) std::exception_ptr(other.area.exception_ptr);
			return *this;
		}
		else if (other.is_ok())
		{
			state = state_type::ok;
			return *this;
		}

		try
		{
			new (&area.error) E(std::move(other.area.error));
			state = state_type::error;
		}
		catch (...)
		{
			new (&area.exception_ptr) std::exception_ptr(std::current_exception());
		}

		return *this;
	}

	~result_base_void() noexcept
	{
		destroy_self();
	}

	bool is_ok() const noexcept { return state == state_type::ok; }
	bool is_error() const noexcept { return state == state_type::error; }
	bool is_exceptional() const noexcept { return state == state_type::exceptional; }

	std::exception_ptr get_exception() const noexcept
	{
		return is_exceptional()
			? area.exception_ptr
			: std::exception_ptr{nullptr};
	}

	const E& unwrap_error() const
	{
		if (is_ok())
			throw bad_result_unwrap(bad_result_unwrap_reason::unwrap_error_in_ok_state);
		else if (is_exceptional())
			throw bad_result_unwrap(bad_result_unwrap_reason::unwrap_in_exceptional_state);
		return area.error;
	}
	
	result_base_void() = delete;
	result_base_void(const result_base_void&) = delete;
	result_base_void& operator=(const result_base_void&) = delete;
};

CEPO_IMPL_NAMESPACE_END;

#endif //CEPO_RESULT_RESULT_BASE_HPP
