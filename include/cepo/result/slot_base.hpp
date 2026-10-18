//Copyright (c) 2026 CB604BL
#ifndef CEPO_RESULT_SLOT_BASE_HPP
#define CEPO_RESULT_SLOT_BASE_HPP

#include "cepo/configs/namespace_macro.h"
#include "cepo/result/exception_chooser.hpp"
#include "cepo/result/exception_state.hpp"
#include "cepo/result/forward_declaration.hpp"		// IWYU pragma: keep
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
		friend class ::cepo::result;

		//Need an uninitialized area
		union area_type
		{
			T slot;
			area_type() noexcept {}
			~area_type() noexcept {}
		};

		area_type area;
		std::exception_ptr exception_ptr;

	public:
		result_slot_base(const result_slot_base&) = delete;
		result_slot_base& operator=(const result_slot_base&) = delete;
	};
}

template<typename T>
using result_value_type = impl::result_slot_base<T, impl::result_slot_value_tag>;

template<typename E>
using result_error_type = impl::result_slot_base<E, impl::result_slot_error_tag>;

CEPO_NAMESPACE_END;

#endif //CEPO_RESULT_SLOT_BASE_HPP
