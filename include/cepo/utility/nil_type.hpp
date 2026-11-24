//Copyright (c) 2026 CB604BL
//CEPO - C++ Eleven Patches Open-source
#ifndef CEPO_UTILITY_NIL_TYPE_HPP
#define CEPO_UTILITY_NIL_TYPE_HPP

#include "cepo/configs/namespace_macro.hpp"

CEPO_NAMESPACE_START;

//Generic empty-state type, the counterpart of std::monostate. All instances
//compare equal: ==, <= and >= are always true, !=, < and > are always false.
//Value semantics only: use it as a constant, do not take its address
//(cepo::nil has internal linkage, so each translation unit owns a copy).
class nil_type
{
public:
	constexpr nil_type() noexcept {}
	constexpr nil_type(const nil_type&) noexcept = default;
	constexpr nil_type(nil_type&&) noexcept = default;
	nil_type& operator=(const nil_type&) noexcept = default;
	nil_type& operator=(nil_type&&) noexcept = default;
	~nil_type() noexcept = default;
};

constexpr bool operator==(
	const nil_type&,
	const nil_type&) noexcept
{ return true; }

constexpr bool operator!=(
	const nil_type&,
	const nil_type&) noexcept
{ return false; }

constexpr bool operator<(
	const nil_type&,
	const nil_type&) noexcept
{ return false; }

constexpr bool operator>(
	const nil_type&,
	const nil_type&) noexcept
{ return false; }

constexpr bool operator<=(
	const nil_type&,
	const nil_type&) noexcept
{ return true; }

constexpr bool operator>=(
	const nil_type&,
	const nil_type&) noexcept
{ return true; }

constexpr nil_type nil = nil_type{};

CEPO_NAMESPACE_END;

#endif //CEPO_UTILITY_NIL_TYPE_HPP
