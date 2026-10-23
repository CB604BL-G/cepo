//Copyright (c) 2026 CB604BL
#ifndef CEPO_RESULT_SLOT_BASE_HPP
#define CEPO_RESULT_SLOT_BASE_HPP

#include "cepo/configs/namespace_macro.h"
#include "cepo/result/base_forward_declaration.hpp"		// IWYU pragma: keep
#include "cepo/result/slot_tags.hpp"
#include <exception>
#include <new>		// IWYU pragma: keep
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
		explicit result_slot_base() noexcept:
			area(),
			exception_ptr(nullptr),
			is_empty(true)
		{
			try
			{
				new (&area.slot) T();
				is_empty = false;
			}
			catch (...)
			{
				exception_ptr = std::current_exception();
			}
		}

		template<typename... Args>
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
	class result_slot_base<void, Tag> {};

	template<typename Tag>
	class result_slot_base<const void, Tag>{};

	template<typename Tag>
	class result_slot_base<volatile void, Tag>{};

	template<typename Tag>
	class result_slot_base<const volatile void, Tag>{};
}

template<typename T>
class result_value_type
	: private impl::result_slot_base<T, impl::result_slot_value_tag>
{
private:
	template<typename Tp, typename Ep>
	friend class ::cepo::impl::result_base;
	
public:
	using impl::result_slot_base<T, impl::result_slot_value_tag>::result_slot_base;
	using impl::result_slot_base<T, impl::result_slot_value_tag>::operator=;
};

template<typename E>
class result_error_type
	: private impl::result_slot_base<E, impl::result_slot_error_tag>
{
private:
	template<typename Tp, typename Ep>
	friend class ::cepo::impl::result_base;
	
	template<typename Void, typename Ep>
	friend class ::cepo::impl::result_base_void;
public:
	using impl::result_slot_base<E, impl::result_slot_error_tag>::result_slot_base;
	using impl::result_slot_base<E, impl::result_slot_error_tag>::operator=;
};

CEPO_NAMESPACE_END;

#endif //CEPO_RESULT_SLOT_BASE_HPP
