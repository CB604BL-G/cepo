//Copyright (c) 2026 CB604BL
//CEPO - C++ Eleven Patches Open-source
#ifndef CEPO_RESULT_SLOT_BASE_HPP
#define CEPO_RESULT_SLOT_BASE_HPP

#include "cepo/configs/namespace_macro.h"
#include "cepo/result/base_forward_declaration.hpp"		// IWYU pragma: keep
#include <exception>
#include <new>											// IWYU pragma: keep
#include <type_traits>
#include <utility>

CEPO_IMPL_NAMESPACE_START;

template<typename T, typename Tag>
class result_slot_base
{
private:
	template<typename Tp, typename Ep>
	friend class ::cepo::impl::result_base;

	template<typename Void, typename Ep>
	friend class ::cepo::impl::result_base_void;

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
			new (&area.slot) T(std::forward<Args>(args)...);
			is_successful = true;
		}
		catch (...)
		{
			new (&area.exception_ptr) std::exception_ptr(std::current_exception());
		}
	}

	result_slot_base(result_slot_base&& other) noexcept:
		area(),
		is_successful(false)
	{
		if (not other.is_successful)
		{
			//Sharing exception instead of moving
			new (&area.exception_ptr) std::exception_ptr(other.area.exception_ptr);
			return;
		}
		
		try
		{
			new (&area.slot) T(std::move(other.area.slot));
			is_successful = true;
		}
		catch (...)
		{
			new (&area.exception_ptr) std::exception_ptr(std::current_exception());
		}

		//The original object remains unchanged,
		//and the stored value becomes valid but unspecified
	}

	result_slot_base& operator=(result_slot_base&& other) noexcept
	{
		if (this == &other) return *this;

		destroy_self();
		is_successful = false;
		if (not other.is_successful)
		{
			new (&area.exception_ptr) std::exception_ptr(other.area.exception_ptr);
			return *this;
		}

		try
		{
			new (&area.slot) T(std::move(other.area.slot));
			is_successful = true;
		}
		catch (...)
		{
			new (&area.exception_ptr) std::exception_ptr(std::current_exception());
		}
		
		return *this;
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

	result_slot_base(const result_slot_base&) = delete;
	result_slot_base& operator=(const result_slot_base&) = delete;
};

CEPO_IMPL_NAMESPACE_END;

#endif //CEPO_RESULT_SLOT_BASE_HPP
