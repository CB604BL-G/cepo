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

CEPO_NAMESPACE_START;

namespace impl
{
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
			area_type() noexcept {}
			~area_type() noexcept {}
		};

		area_type area;
		std::exception_ptr exception_ptr;
		//If exception_ptr != nullptr
		//is_empty = true
		bool is_empty;

		void destroy_self() noexcept
		{
			if (not is_empty)
			{
				area.slot.~T();
				is_empty = true;
			}
			exception_ptr = nullptr;
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
			exception_ptr(nullptr),
			is_empty(true)
		{
			try
			{
				new (&area.slot) T(std::forward<Args>(args)...);
				is_empty = false;
			}
			catch (...)
			{
				exception_ptr = std::current_exception();
			}
		}

		result_slot_base(result_slot_base&& other) noexcept:
			area(),
			exception_ptr(nullptr),
			is_empty(true)
		{
			if (other.is_empty)
			{
				exception_ptr = other.exception_ptr;
				other.exception_ptr = nullptr;
				return;
			}
			
			try
			{
				new (&area.slot) T(std::move(other.area.slot));
				is_empty = false;
			}
			catch (...)
			{
				exception_ptr = std::current_exception();
			}

			//Intentionally not putting it in the try-catch
			other.destroy_self();
		}

		result_slot_base& operator=(result_slot_base&& other) noexcept
		{
			if (this == &other) return *this;

			destroy_self();
			if (other.is_empty)
			{
				exception_ptr = other.exception_ptr;
				other.exception_ptr = nullptr;
				return *this;
			}

			try
			{
				new (&area.slot) T(std::move(other.area.slot));
				is_empty = false;
			}
			catch (...)
			{
				exception_ptr = std::current_exception();
			}

			other.destroy_self();
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
}

CEPO_NAMESPACE_END;

#endif //CEPO_RESULT_SLOT_BASE_HPP
