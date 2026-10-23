//Copyright (c) 2026 CB604BL
#ifndef CEPO_RESULT_RESULT_BASE_HPP
#define CEPO_RESULT_RESULT_BASE_HPP

#include "cepo/configs/namespace_macro.h"
#include "cepo/result/bad_result_unwrap.hpp"
#include "cepo/result/slot_base.hpp"
#include <exception>
#include <utility>

CEPO_NAMESPACE_START;

namespace impl
{
	template<typename T, typename E>
	class result_base
	{
	private:
		union area_type
		{
			T value;
			E error;
			area_type() noexcept {}
			~area_type() noexcept {}
		};

		enum class state_type : unsigned char
		{
			ok,
			error,
			empty		//For T/E construction failed and result's moved-form
		};

		area_type area;
		//Exceptions are only used as auxiliary information,
		//but in fact, they are still three states
		std::exception_ptr exception_ptr;
		state_type state;

		void destroy_self() noexcept
		{
			if (state == state_type::ok)
				area.value.~T();
			else if (state == state_type::error)
				area.error.~E();

			state = state_type::empty;
			exception_ptr = nullptr;
		}

	protected:
		inline T& get_value_unchecked() noexcept { return area.value; }
		inline const T& get_value_unchecked() const noexcept { return area.value; }

		//const-only
		inline const E& get_error_unchecked() const noexcept { return area.error; }
	
	public:
		result_base(result_base&& other) noexcept:
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
				exception_ptr = std::current_exception();
			}
	
			other.destroy_self();
		}
	
		result_base(result_value_type<T>&& value) noexcept:
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
				state = state_type::ok;
			}
			catch (...)
			{
				exception_ptr = std::current_exception();
			}
	
			value.destroy_self();
		}

		result_base(result_error_type<E>&& error) noexcept:
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
				state = state_type::error;
			}
			catch (...)
			{
				exception_ptr = std::current_exception();
			}
	
			error.destroy_self();
		}
	
		result_base& operator=(result_base&& other) noexcept
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
				exception_ptr = std::current_exception();
			}
	
			other.destroy_self();
	
			return *this;
		}
	
		~result_base() noexcept
		{
			destroy_self();
		}
	
		bool is_ok() const noexcept { return state == state_type::ok; }
		bool is_error() const noexcept { return state == state_type::error; }
		bool is_empty() const noexcept { return state == state_type::empty; }
		bool has_exception() const noexcept { return exception_ptr != nullptr; }

		std::exception_ptr get_exception() const noexcept { return exception_ptr; }

		T& unwrap_value()
		{
			if (is_error())
				throw bad_result_unwrap(bad_result_unwrap::reason_type::is_error);
			else if (is_empty())
				throw bad_result_unwrap(bad_result_unwrap::reason_type::is_empty);
			return area.value;
		}
		
		const T& unwrap_value() const
		{
			if (is_error())
				throw bad_result_unwrap(bad_result_unwrap::reason_type::is_error);
			else if (is_empty())
				throw bad_result_unwrap(bad_result_unwrap::reason_type::is_empty);
			return area.value;
		}

		const E& unwrap_error() const
		{
			if (is_ok())
				throw bad_result_unwrap(bad_result_unwrap::reason_type::is_ok);
			else if (is_empty())
				throw bad_result_unwrap(bad_result_unwrap::reason_type::is_empty);
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
			area_type() noexcept {};
			~area_type() noexcept {};
		};

		enum class state_type : unsigned char
		{
			ok,
			error,
			empty
		};

		area_type area;
		std::exception_ptr exception_ptr;
		state_type state;

		void destroy_self() noexcept
		{
			//No need to destruct void
			if (state == state_type::error)
				area.error.~E();
			state = state_type::empty;
			exception_ptr = nullptr;
		}

	protected:
		inline const E& get_error_unchecked() const noexcept { return area.error; }
		
	public:
		result_base_void(result_base_void&& other) noexcept:
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
			else if (other.is_ok())
			{
				state = state_type::ok;
				other.state = state_type::empty;
				return;
			}

			try
			{
				new (&area.error) E(std::move(other.area.error));
				state = state_type::error;
			}
			catch (...)
			{
				exception_ptr = std::current_exception();
			}

			other.destroy_self();
		}

		//There is NO data member in result_value_type when std::is_void<Void>::value is true
		result_base_void(result_value_type<Void>&&) noexcept:
			area(),
			exception_ptr(nullptr),
			state(state_type::ok)
		{}

		result_base_void(result_error_type<E>&& error) noexcept:
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
				state = state_type::error;
			}
			catch (...)
			{
				exception_ptr = std::current_exception();
			}

			error.destroy_self();
		}

		result_base_void& operator=(result_base_void&& other) noexcept
		{
			if (this == &other) return *this;

			destroy_self();

			if (other.is_empty())
			{
				exception_ptr = other.exception_ptr;
				other.exception_ptr = nullptr;
				return *this;
			}
			else if (other.is_ok())
			{
				state = state_type::ok;
				other.state = state_type::empty;
				return *this;
			}

			try
			{
				new (&area.error) E(std::move(other.area.error));
				state = state_type::error;
			}
			catch (...)
			{
				exception_ptr = std::current_exception();
			}

			other.destroy_self();
			return *this;
		}

		~result_base_void() noexcept
		{
			destroy_self();
		}

		bool is_ok() const noexcept { return state == state_type::ok; }
		bool is_error() const noexcept { return state == state_type::error; }
		bool is_empty() const noexcept { return state == state_type::empty; }
		bool has_exception() const noexcept { return exception_ptr != nullptr; }

		std::exception_ptr get_exception() const noexcept { return exception_ptr; }

		const E& unwrap_error() const
		{
			if (is_ok())
				throw bad_result_unwrap(bad_result_unwrap::reason_type::is_ok);
			else if (is_empty())
				throw bad_result_unwrap(bad_result_unwrap::reason_type::is_empty);
			return area.error;
		}
		
		result_base_void() = delete;
		result_base_void(const result_base_void&) = delete;
		result_base_void& operator=(const result_base_void&) = delete;
	};
}

CEPO_NAMESPACE_END;

#endif //CEPO_RESULT_RESULT_BASE_HPP
