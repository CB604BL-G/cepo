//Copyright (c) 2026 CB604BL
#ifndef CEPO_RESULT_BASE_VOID_HELPER_HPP
#define CEPO_RESULT_BASE_VOID_HELPER_HPP

#include "cepo/callable/call_result.hpp"
#include "cepo/callable/is_callable.hpp"
#include "cepo/configs/namespace_macro.h"
#include "cepo/macros/nodiscard.hpp"
#include "cepo/result/result_base.hpp"
#include "cepo/result/when_error.hpp"
#include "cepo/type_traits/conjunction.hpp"
#include <cstdarg>
#include <type_traits>
#include <utility>

CEPO_NAMESPACE_START;

namespace impl
{
	template<typename Void, typename E>
	class result_base_void_helper
		: private result_base_void<Void, E>
	{
	private:
		using base_type = result_base_void<Void, E>;

	public:
		using result_base_void<Void, E>::result_base_void;

		//Forward constructor
		template<
			typename... Args,
			typename std::enable_if<
				std::is_constructible<base_type, Args...>::value and
				not conjunction<std::is_same<result_base_void_helper,
					typename std::remove_cv<
						typename std::remove_reference<Args>::type
					>::type>...>::value,
			int>::type = 0>
		result_base_void_helper(Args&&... args) noexcept:
			base_type::result_base_void(std::forward<Args>(args)...)
		{}

		using base_type::is_ok;
		using base_type::is_error;
		using base_type::is_empty;
		using base_type::has_exception;

		using base_type::get_exception;
		using base_type::unwrap_error;

		template<typename F>
		CEPO_NODISCARD
		auto ok(F&& callback) const
			-> typename std::enable_if<
					is_callable<F>::value and
					std::is_void<
						typename ::cepo::call_result<F>::type>::value,
					result_when_error<E>
				>::type
		{
			if (is_ok())
				call(std::forward<F>(callback));
			return result_when_error<E>{
				is_error() ?
				&this->get_error_unchecked() :
				nullptr
			};
		}
	};
}

CEPO_NAMESPACE_END;

#endif //CEPO_RESULT_BASE_VOID_HELPER_HPP
