//Copyright (c) 2026 CB604BL
//CEPO - C++ Eleven Patches Open-source
#ifndef CEPO_RESULT_SLOT_BASE_HPP
#define CEPO_RESULT_SLOT_BASE_HPP

#include "cepo/configs/namespace_macro.hpp"
#include <exception>
#include <new>											// IWYU pragma: keep
#include <type_traits>
#include <utility>

CEPO_IMPL_NAMESPACE_START;

template<typename T, typename Tag>
class result_slot_base
{
private:
	//Need an uninitialized area
	union area_type
	{
		T slot;
		std::exception_ptr exception_ptr;
		
		area_type() noexcept {}
		~area_type() noexcept {}
	};

	area_type area;
	bool is_successful;

	template<typename... Args>
	void construct_T(Args&&... args)
		noexcept(std::is_nothrow_constructible<T, Args...>::value)
	{
		new (&area.slot) T(std::forward<Args>(args)...);
		is_successful = true;
	}

	template<typename... Args>
	void construct_exception_ptr(Args&&... args)
		noexcept(std::is_nothrow_constructible<std::exception_ptr, Args...>::value)
	{
		new (&area.exception_ptr) std::exception_ptr(std::forward<Args>(args)...);
		is_successful = false;
	}

	void destroy_self() noexcept
	{
		if (is_successful)
			area.slot.~T();
		else
		{
			area.exception_ptr.~decltype(area.exception_ptr)();
		}
		//Keep current state
	}

	void destroy_to_useless() noexcept
	{
		destroy_self();
		is_successful = false;
	}

public:
	template<
		typename... Args,
		typename std::enable_if<
			std::is_constructible<T, Args...>::value,
			int
		>::type = 0>
	explicit result_slot_base(Args&&... args) noexcept:
		area(),
		is_successful(false)
	{
		try
		{
			construct_T(std::forward<Args>(args)...);
		}
		catch (...)
		{
			construct_exception_ptr(std::current_exception());
		}
	}

	result_slot_base(result_slot_base&& other) noexcept:
		area(),
		is_successful(false)
	{
		if (not other.is_successful)
		{
			//Sharing exception instead of moving
			construct_exception_ptr(other.area.exception_ptr);
			return;
		}
		
		try
		{
			construct_T(std::move(other.area.slot));
		}
		catch (...)
		{
			construct_exception_ptr(std::current_exception());
		}

		//The original object remains unchanged,
		//and the stored value becomes valid but unspecified
	}

	result_slot_base& operator=(result_slot_base&& other) noexcept
	{
		if (this == &other) return *this;

		destroy_to_useless();
		if (not other.is_successful)
		{
			construct_exception_ptr(other.area.exception_ptr);
			return *this;
		}

		try
		{
			construct_T(std::move(other.area.slot));
		}
		catch (...)
		{
			construct_exception_ptr(std::current_exception());
		}
		
		return *this;
	}

	//Unchecked: dereferencing an exceptional slot reads the dead branch
	//of the union, which is undefined behavior. Check is_exceptional()
	//first; cepo::result::unwrap_value is the checked path.
	T& operator*() noexcept
	{
		return area.slot;
	}

	const T& operator*() const noexcept
	{
		return area.slot;
	}

	bool is_exceptional() const noexcept
	{
		return not is_successful;
	}

	std::exception_ptr get_exception() const noexcept
	{
		return is_exceptional()
			? area.exception_ptr
			: std::exception_ptr{nullptr};
	}

	~result_slot_base() noexcept
	{
		destroy_self();
	}
	
	result_slot_base(const result_slot_base&) = delete;
	result_slot_base& operator=(const result_slot_base&) = delete;
};

template<typename Tag>
class result_slot_base<void, Tag>
{
public:
	explicit result_slot_base() noexcept = default;
	result_slot_base(result_slot_base&&) noexcept = default;
	result_slot_base& operator=(result_slot_base&&) noexcept = default;
	~result_slot_base() noexcept = default;

	bool is_exceptional() const noexcept { return false; }
	std::exception_ptr get_exception() const noexcept { return std::exception_ptr{nullptr}; }

	result_slot_base(const result_slot_base&) = delete;
	result_slot_base& operator=(const result_slot_base&) = delete;
};

template<typename Tag>
class result_slot_base<const void, Tag>
{
public:
	explicit result_slot_base() noexcept = default;
	result_slot_base(result_slot_base&&) noexcept = default;
	result_slot_base& operator=(result_slot_base&&) noexcept = default;
	~result_slot_base() noexcept = default;

	bool is_exceptional() const noexcept { return false; }
	std::exception_ptr get_exception() const noexcept { return std::exception_ptr{nullptr}; }

	result_slot_base(const result_slot_base&) = delete;
	result_slot_base& operator=(const result_slot_base&) = delete;
};

template<typename Tag>
class result_slot_base<volatile void, Tag>
{
public:
	explicit result_slot_base() noexcept = default;
	result_slot_base(result_slot_base&&) noexcept = default;
	result_slot_base& operator=(result_slot_base&&) noexcept = default;
	~result_slot_base() noexcept = default;

	bool is_exceptional() const noexcept { return false; }
	std::exception_ptr get_exception() const noexcept { return std::exception_ptr{nullptr}; }

	result_slot_base(const result_slot_base&) = delete;
	result_slot_base& operator=(const result_slot_base&) = delete;
};

template<typename Tag>
class result_slot_base<const volatile void, Tag>
{
public:
	explicit result_slot_base() noexcept = default;
	result_slot_base(result_slot_base&&) noexcept = default;
	result_slot_base& operator=(result_slot_base&&) noexcept = default;
	~result_slot_base() noexcept = default;

	bool is_exceptional() const noexcept { return false; }
	std::exception_ptr get_exception() const noexcept { return std::exception_ptr{nullptr}; }

	result_slot_base(const result_slot_base&) = delete;
	result_slot_base& operator=(const result_slot_base&) = delete;
};

CEPO_IMPL_NAMESPACE_END;

#endif //CEPO_RESULT_SLOT_BASE_HPP
