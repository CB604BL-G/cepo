//Copyright (c) 2026 CB604BL
//CEPO - C++ Eleven Patches Open-source
#ifndef CEPO_MEMORY_MAKE_UNIQUE_HPP
#define CEPO_MEMORY_MAKE_UNIQUE_HPP

#include "cepo/configs/namespace_macro.h"
#include "cepo/macros/nodiscard.hpp"
#include "cepo/type_traits/is_bounded_array.hpp"
#include "cepo/type_traits/is_unbounded_array.hpp"
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

CEPO_NAMESPACE_START;

/*T is not an array*/
template<typename T, typename... Args>
CEPO_NODISCARD
typename std::enable_if<
	not std::is_array<T>::value and
	std::is_constructible<T, Args...>::value,
	std::unique_ptr<T>>::type
make_unique(Args&&... args)
{
	return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

/*T is an unbounded array*/
template<typename T>
CEPO_NODISCARD
typename std::enable_if<
	is_unbounded_array<T>::value and
	std::is_default_constructible<
		typename std::remove_extent<T>::type>::value,
	std::unique_ptr<T>>::type
make_unique(std::size_t size)
{
	return std::unique_ptr<T>(new typename std::remove_extent<T>::type[size]());
}

//I added extended support for bounded arrays to cepo::make_unique;
//std::make_unique doesn't support this feature.

//That is to say, you can now call it in a form such as
//auto i = cepo::make_unique<int[5]>();
//But note that its return type is still std::unique_ptr<int[]>
//rather than std::unique_ptr<int[5]>. The latter is a “crippled type,”
//so you need to be aware of this when using it.
template<typename T>
CEPO_NODISCARD
typename std::enable_if<
	is_bounded_array<T>::value and
	std::is_default_constructible<
		typename std::remove_extent<T>::type>::value,
	std::unique_ptr<
		typename std::remove_extent<T>::type[]>>::type
make_unique()
{
	return std::unique_ptr<
		typename std::remove_extent<T>::type[]>(new T());
}

CEPO_NAMESPACE_END;

#endif //CEPO_MEMORY_MAKE_UNIQUE_HPP
