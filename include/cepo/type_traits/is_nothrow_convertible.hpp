//Copyright (c) 2026 CB604BL
#ifndef CEPO_TYPE_TRAITS_IS_NOTHROW_CONVERTIBLE_HPP
#define CEPO_TYPE_TRAITS_IS_NOTHROW_CONVERTIBLE_HPP

#include "cepo/configs/namespace_macro.h"
#include <type_traits>
#include <utility>

CEPO_NAMESPACE_START;

namespace impl
{
	template<typename From, typename To,
		bool is_void = std::is_void<To>::value or std::is_void<From>::value>
	struct is_nothrow_convertible
	{
		static constexpr bool value = std::is_void<To>::value and std::is_void<From>::value;
	};

	template<typename From, typename To>
	struct is_nothrow_convertible<From, To, false>
	{
	private:
		template<typename ToF>
		static void test_aux(ToF) noexcept;
	
		template<typename FromT, typename ToT>
		static auto test(int) ->
			decltype(
				static_cast<ToT(*)()>(nullptr),
				std::integral_constant<
					bool,
					noexcept(test_aux<ToT>(std::declval<FromT>()))>{});

		template<typename...>
		static std::false_type test(...);

	public:
		static constexpr bool value = decltype(test<From, To>(1))::value;
	};

	template<typename From, typename To, bool is_void>
	constexpr bool is_nothrow_convertible<From, To, is_void>::value;

	template<typename From, typename To>
	constexpr bool is_nothrow_convertible<From, To, false>::value;
}

template<typename From, typename To>
struct is_nothrow_convertible
{
	static constexpr bool value = impl::is_nothrow_convertible<From, To>::value;
};

template<typename From, typename To>
constexpr bool is_nothrow_convertible<From, To>::value;

CEPO_NAMESPACE_END;

#endif //CEPO_TYPE_TRAITS_IS_NOTHROW_CONVERTIBLE_HPP
