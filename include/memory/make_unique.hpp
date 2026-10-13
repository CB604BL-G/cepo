//Copyright (c) 2026 CB604BL
#ifndef CEPO_MEMORY_MAKE_UNIQUE_HPP
#define CEPO_MEMORY_MAKE_UNIQUE_HPP

#include "configs/namespace_macro.h"
#include "type_traits/is_bounded_array.hpp"
#include "type_traits/is_unbounded_array.hpp"
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

CEPO_NAMESPACE_START;

/*T is not an array*/
template<typename T, typename... Args>
typename std::enable_if<
	not std::is_array<T>::value,
	std::unique_ptr<T>>::type
make_unique(Args&&... args)
{
	static_assert(
		std::is_constructible<T, Args...>::value,
		"cepo::make_unique: T is not constructible from the provided arguments");
	return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

/*T is an unbounded array*/
template<typename T>
typename std::enable_if<
	is_unbounded_array<T>::value,
	std::unique_ptr<T>>::type
make_unique(std::size_t size)
{
	static_assert(
		std::is_default_constructible<
			typename std::remove_extent<T>::type>::value,
		"cepo::make_unique<T[]>: array element type is not default constructible");
	return std::unique_ptr<T>(new typename std::remove_extent<T>::type[size]());
}

/*T is a bounded array*/
template<typename T>
typename std::enable_if<
	is_bounded_array<T>::value,
	std::unique_ptr<
		typename std::remove_extent<T>::type[]>>::type
make_unique()
{
	static_assert(
		std::is_default_constructible<
			typename std::remove_extent<T>::type>::value,
		"cepo::make_unique<T[N]>: array element type is not default constructible");
	return std::unique_ptr<
		typename std::remove_extent<T>::type[]>(new T());
}

CEPO_NAMESPACE_END;

#endif //CEPO_MEMORY_MAKE_UNIQUE_HPP
