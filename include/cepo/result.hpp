//Copyright (c) 2026 CB604BL
#ifndef CEPO_RESULT_HPP
#define CEPO_RESULT_HPP

#include "cepo/configs/namespace_macro.h"
#include <new>		// IWYU pragma: keep
#include <type_traits>
#include <utility>

CEPO_NAMESPACE_START;

template<typename T, typename E>
class result;

enum class result_exception_state
{
	value_construction_exception,
	error_construction_exception,
	no_exception_threw
};

template<typename T>
class result_value_type
{
private:
	static_assert(std::is_nothrow_destructible<T>::value,
		"result_value_type requires T to have a noexcept destructor");
	
	template<typename ValueType, typename E>
	friend class result;
	
	union value_area
	{
		T value;

		value_area() noexcept {}
		~value_area() noexcept {}
	};

	value_area area;
	result_exception_state exception_state;

	void destroy_self() noexcept
	{
		if (exception_state == result_exception_state::no_exception_threw)
		{
			area.value.~T();
		}
	}

public:
	struct value_exception_tag{};
	
	result_value_type() = delete;

	result_value_type(result_value_type&& other) noexcept:
		area(),
		exception_state(other.exception_state)
	{
		if (exception_state == result_exception_state::no_exception_threw)
		{
			try
			{
				new (&area.value) T(std::move(other.area.value));
			}
			catch (...)
			{
				exception_state = result_exception_state::value_construction_exception;
				other.destroy_self();
				other.exception_state = result_exception_state::value_construction_exception;
			}
		}
	}
	
	result_value_type(value_exception_tag) noexcept:
		area(),
		exception_state(result_exception_state::value_construction_exception)
	{};

	result_value_type(T&& value) noexcept:
		area(),
		exception_state(result_exception_state::no_exception_threw)
	{
		try
		{
			new (&area.value) T(std::move(value));
		}
		catch (...)
		{
			exception_state = result_exception_state::value_construction_exception;
		}
	}

	result_value_type& operator=(result_value_type&& other) noexcept
	{
		if (this == &other) return *this;

		destroy_self();
		exception_state = other.exception_state;

		if (exception_state == result_exception_state::no_exception_threw)
		{
			try
			{
				new (&area.value) T(std::move(other.area.value));
			}
			catch (...)
			{
				exception_state = result_exception_state::value_construction_exception;
				other.destroy_self();
				other.exception_state = result_exception_state::value_construction_exception;
			}
		}

		return *this;
	}

	~result_value_type() noexcept
	{
		destroy_self();
	}
};

template<typename T, typename... Args>
result_value_type<T> make_result_value(Args&&... args) noexcept
{
	try
	{
		return result_value_type<T>(T(std::forward<Args>(args)...));
	}
	catch (...)
	{
		return result_value_type<T>(typename result_value_type<T>::value_exception_tag());
	}
}

template<typename E>
class result_error_type
{
private:
	static_assert(std::is_nothrow_destructible<E>::value,
		"result_error_type requires E to have a noexcept destructor");
	
	template<typename T, typename ErrorType>
	friend class result;
	
	union error_area
	{
		E error;

		error_area() noexcept {}
		~error_area() noexcept {}
	};

	error_area area;
	result_exception_state exception_state;

	void destroy_self() noexcept
	{
		if (exception_state == result_exception_state::no_exception_threw)
		{
			area.error.~E();
		}
	}

public:
	struct error_exception_tag{};
	
	result_error_type() = delete;

	result_error_type(result_error_type&& other) noexcept:
		area(),
		exception_state(other.exception_state)
	{
		if (exception_state == result_exception_state::no_exception_threw)
		{
			try
			{
				new (&area.error) E(std::move(other.area.error));
			}
			catch (...)
			{
				exception_state = result_exception_state::error_construction_exception;
				other.destroy_self();
				other.exception_state = result_exception_state::error_construction_exception;
			}
		}
	}
	
	result_error_type(error_exception_tag) noexcept:
		area(),
		exception_state(result_exception_state::error_construction_exception)
	{};

	result_error_type(E&& error) noexcept:
		area(),
		exception_state(result_exception_state::no_exception_threw)
	{
		try
		{
			new (&area.error) E(std::move(error));
		}
		catch (...)
		{
			exception_state = result_exception_state::error_construction_exception;
		}
	}

	result_error_type& operator=(result_error_type&& other) noexcept
	{
		if (this == &other) return *this;

		destroy_self();
		exception_state = other.exception_state;

		if (exception_state == result_exception_state::no_exception_threw)
		{
			try
			{
				new (&area.error) E(std::move(other.area.error));
			}
			catch (...)
			{
				exception_state = result_exception_state::error_construction_exception;
				other.destroy_self();
				other.exception_state = result_exception_state::error_construction_exception;
			}
		}

		return *this;
	}

	~result_error_type() noexcept
	{
		destroy_self();
	}
};

template<typename E, typename... Args>
result_error_type<E> make_result_error(Args&&... args) noexcept
{
	try
	{
		return result_error_type<E>(E(std::forward<Args>(args)...));
	}
	catch (...)
	{
		return result_error_type<E>(typename result_error_type<E>::error_exception_tag());
	}
}

template<typename T, typename E>
class result
{
private:
	enum class stored_state : unsigned char
	{
		has_value,
		has_error,
		empty
	};

	class when_error
	{
	private:
		result* handler;
		
	public:
		when_error(result* handler) noexcept:
			handler(handler)
		{}
		
		template<typename F>
		void error(F&& f)
		{
			if (handler->has_error())
				f(handler->area.error);
		}
	};

	union area_type
	{
		T value;
		E error;

		area_type() noexcept {};
		~area_type() noexcept {};
	};

	area_type area;
	stored_state stored;
	result_exception_state exception_state;

	void destroy_self() noexcept
	{
		switch (stored)
		{
		case stored_state::has_value:
			area.value.~T();
			break;
		case stored_state::has_error:
			area.error.~E();
			break;
		case stored_state::empty:
			break;
		}
		stored = stored_state::empty;
	}

public:
	result() noexcept = delete;

	bool has_value() const noexcept { return stored == stored_state::has_value; }
	bool has_error() const noexcept { return stored == stored_state::has_error; }
	bool is_empty() const noexcept { return stored == stored_state::empty; }

	template<typename F>
	when_error ok(F&& f)
	{
		if (has_value())
			f(area.value);
		return when_error{this};
	}

	result(result&& other) noexcept:
		area(),
		stored(other.stored),
		exception_state(other.exception_state)
	{
		switch (stored)
		{
		case stored_state::has_value:
			try
			{
				new (&area.value) T(std::move(other.area.value));
				other.destroy_self();
			}
			catch (...)
			{
				exception_state = result_exception_state::value_construction_exception;
				stored = stored_state::empty;
				other.destroy_self();
				other.exception_state = result_exception_state::value_construction_exception;
			}
			break;

		case stored_state::has_error:
			try
			{
				new (&area.error) E(std::move(other.area.error));
				other.destroy_self();
			}
			catch (...)
			{
				exception_state = result_exception_state::error_construction_exception;
				stored = stored_state::empty;
				other.destroy_self();
				other.exception_state = result_exception_state::error_construction_exception;
			}
			break;

		case stored_state::empty:
			break;
		}
	}

	result(result_value_type<T>&& value) noexcept:
		area(),
		stored(value.exception_state == result_exception_state::no_exception_threw
			? stored_state::has_value
			: stored_state::empty),
		exception_state(value.exception_state)
	{
		if (stored != stored_state::has_value)
			return;

		try
		{
			new (&area.value) T(std::move(value.area.value));
		}
		catch (...)
		{
			exception_state = result_exception_state::value_construction_exception;
			stored = stored_state::empty;
			value.destroy_self();
			value.exception_state = result_exception_state::value_construction_exception;
		}
	}

	result(result_error_type<E>&& error) noexcept:
		area(),
		stored(error.exception_state == result_exception_state::no_exception_threw
			? stored_state::has_error
			: stored_state::empty),
		exception_state(error.exception_state)
	{
		if (stored != stored_state::has_error)
			return;

		try
		{
			new (&area.error) E(std::move(error.area.error));
		}
		catch (...)
		{
			exception_state = result_exception_state::error_construction_exception;
			stored = stored_state::empty;
			error.destroy_self();
			error.exception_state = result_exception_state::error_construction_exception;
		}
	}

	result& operator=(result&& other) noexcept
	{
		if (this == &other) return *this;

		destroy_self();
		stored = other.stored;
		exception_state = other.exception_state;

		switch (stored)
		{
		case stored_state::has_value:
			try
			{
				new (&area.value) T(std::move(other.area.value));
				other.destroy_self();
			}
			catch (...)
			{
				exception_state = result_exception_state::value_construction_exception;
				stored = stored_state::empty;
				other.destroy_self();
				other.exception_state = result_exception_state::value_construction_exception;
			}
			break;

		case stored_state::has_error:
			try
			{
				new (&area.error) E(std::move(other.area.error));
				other.destroy_self();
			}
			catch (...)
			{
				exception_state = result_exception_state::error_construction_exception;
				stored = stored_state::empty;
				other.destroy_self();
				other.exception_state = result_exception_state::error_construction_exception;
			}
			break;

		case stored_state::empty:
			break;
		}

		return *this;
	}

	~result() noexcept
	{
		destroy_self();
	}
};

CEPO_NAMESPACE_END;

#endif //CEPO_RESULT_HPP
