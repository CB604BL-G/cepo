//Copyright (c) 2026 CB604BL
#ifndef CEPO_MEMORY_MAKE_UNIQUE_HPP
#define CEPO_MEMORY_MAKE_UNIQUE_HPP

#include "configs/namespace_macro.h"
#include "type_traits/is_bounded_array.hpp"
#include "utility/always_false.hpp"
#include "utility/arg_pack_tuple.hpp"
#include "utility/arg_pack_tuple_type_getter_with_always_type.hpp"
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

CEPO_NAMESPACE_START;

/*T is not an array*/
template<typename T, typename... Args>
typename std::enable_if<
	not std::is_array<T>::value and
	std::is_constructible<T, Args...>::value,
	std::unique_ptr<T>>::type
make_unique(Args&&... args)
{
	return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

//When T cannot be constructed
template<typename T, typename... Args>
typename std::enable_if<
	not std::is_array<T>::value and
	not std::is_constructible<T, Args...>::value,
	std::unique_ptr<T>>::type
make_unique(Args&&...)
{
	static_assert(
		always_false<T, Args...>::value,
		"cepo::make_unique: T is not constructible from the provided arguments");
}

/*T is an unbounded array*/
template<typename T>
typename std::enable_if<
	is_unbounded_array<T>::value and
	std::is_default_constructible<
		typename std::remove_extent<T>::type>::value,
	std::unique_ptr<T>>::type
make_unique(std::size_t size)
{
	return std::unique_ptr<T>(new typename std::remove_extent<T>::type[size]());
}

//When T is an unbounded array and the argument is not a valid size
template<typename T, typename... Args>
typename std::enable_if<
	is_unbounded_array<T>::value and
	std::is_default_constructible<
		typename std::remove_extent<T>::type>::value and
	(sizeof...(Args) != 1 or
	not std::is_convertible<
		typename utility::arg_pack_tuple_type_getter_with_always_type<arg_pack_tuple<Args...>, 0>::type,
		std::size_t>::value),
	std::unique_ptr<T>>::type
make_unique(Args&&...)
{
	static_assert(
		always_false<T, Args...>::value,
		"cepo::make_unique<T[]>: array version requires exactly one argument "
		"convertible to std::size_t");
}

//When the element type of an unbounded array is not default constructible
template<typename T, typename... Args>
typename std::enable_if<
	is_unbounded_array<T>::value and
	not std::is_default_constructible<
		typename std::remove_extent<T>::type>::value and
	(sizeof...(Args) == 1 and
	std::is_convertible<
		typename utility::arg_pack_tuple_type_getter_with_always_type<arg_pack_tuple<Args...>, 0>::type,
		std::size_t>::value),
	std::unique_ptr<T>>::type
make_unique(Args&&...)
{
	static_assert(
		always_false<T, Args...>::value,
		"cepo::make_unique<T[]>: array element type is not default constructible");
}

//When the element type of an unbounded array is not default constructible
//and the argument is not a valid size
template<typename T, typename... Args>
typename std::enable_if<
	is_unbounded_array<T>::value and
	not std::is_default_constructible<
		typename std::remove_extent<T>::type>::value and
	(sizeof...(Args) != 1 or
	not std::is_convertible<
		typename utility::arg_pack_tuple_type_getter_with_always_type<arg_pack_tuple<Args...>, 0>::type,
		std::size_t>::value),
	std::unique_ptr<T>>::type
make_unique(Args&&...)
{
	static_assert(
		always_false<T, Args...>::value,
		"cepo::make_unique<T[]>: array element type is not default constructible, "
		"and the argument is not a valid size");
}

/*T is a bounded array*/
template<typename T>
typename std::enable_if<
	is_bounded_array<T>::value and
	std::is_default_constructible<
		typename std::remove_extent<T>::type>::value,
	std::unique_ptr<T>>::type
make_unique()
{
	return std::unique_ptr<T>(new typename std::remove_extent<T>::type[std::extent<T, 0>::value]());
}

//When T is a bounded array but extra arguments are provided
template<typename T, typename... Args>
typename std::enable_if<
	is_bounded_array<T>::value and
	std::is_default_constructible<
		typename std::remove_extent<T>::type>::value and
	sizeof...(Args) != 0,
	std::unique_ptr<T>>::type
make_unique(Args&&...)
{
	static_assert(
		always_false<T, Args...>::value,
		"cepo::make_unique<T[N]>: bounded array version does not accept a size argument");
}

//When the element type of a bounded array is not default constructible
template<typename T, typename... Args>
typename std::enable_if<
	is_bounded_array<T>::value and
	not std::is_default_constructible<
		typename std::remove_extent<T>::type>::value and
	sizeof...(Args) == 0,
	std::unique_ptr<T>>::type
make_unique(Args&&...)
{
	static_assert(
		always_false<T, Args...>::value,
		"cepo::make_unique<T[N]>: array element type is not default constructible");
}

//When the element type of a bounded array is not default constructible
//and extra arguments are provided
template<typename T, typename... Args>
typename std::enable_if<
	is_bounded_array<T>::value and
	not std::is_default_constructible<
		typename std::remove_extent<T>::type>::value and
	sizeof...(Args) != 0,
	std::unique_ptr<T>>::type
make_unique(Args&&...)
{
	static_assert(
		always_false<T, Args...>::value,
		"cepo::make_unique<T[N]>: array element type is not default constructible, "
		"and bounded array version does not accept a size argument");
}

CEPO_NAMESPACE_END;

#endif //CEPO_MEMORY_MAKE_UNIQUE_HPP
