//Copyright (c) 2026 CB604BL
//CEPO - C++ Eleven Patches Open-source
#ifndef CEPO_DEFER_HPP
#define CEPO_DEFER_HPP

#include "cepo/configs/namespace_macro.hpp"
#include <utility>

//Reference Project: https://github.com/UN-chief/CPP_Defer

//How to use this file.

//CEPO_DEFER is the only supported way in. It declares a defer whose callback is
//the block written right after it, and that callback runs when the enclosing
//scope ends, so several of them in one scope run in reverse order.

//The block is a void lambda, so returning a value from it is a compile error;
//a bare return only leaves the block, not the surrounding function.

//A defer belongs in a block, not at namespace scope. Outside a function the
//capture default is ill-formed; and even where it compiled, the name it
//declares would be a reference at namespace scope, and a reference keeps
//external linkage, so a header included by two translation units would end
//in a duplicate symbol at link time.

//Write one per line as well. The index macro falls back to __LINE__ where
//__COUNTER__ is missing, so two of them sharing a line declare one name.

//Reaching a defer any other way is undefined behavior: naming its type,
//calling defer_maker or operator+ yourself, copying it, moving it into a
//variable of your own, storing it for later. The class and the maker are
//implementation details; they exist to carry the macro and they promise
//nothing to a hand written call site.

//The callback runs inside a destructor, so it must not throw. CEPO_DEFER
//makes its lambda noexcept for that reason: a throwing callback reaches a
//destructor that cannot pass the exception on, and the program terminates.
CEPO_IMPL_NAMESPACE_START;

template<typename F>
class defer
{
private:
	F callback;

public:
	template<typename FF>
	defer(FF&& f) noexcept:
		callback(std::forward<FF>(f))
	{}

	//The destructor below suppresses the implicit move constructor and the copy
	//constructor is deleted, so this one has to be written out: returning a
	//defer from the maker needs an accessible move constructor even when the
	//result is elided.
	defer(defer&&) noexcept = default;

	~defer() noexcept
	{
		callback();
	}

	defer() = delete;
	defer(const defer&) = delete;
	defer& operator=(const defer&) = delete;
	defer& operator=(defer&&) = delete;
};

struct defer_maker
{
	template<typename F>
	defer<F> operator+(F&& callback) noexcept
	{
		return defer<F>(std::forward<F>(callback));
	}
};

CEPO_IMPL_NAMESPACE_END;

#define CEPO_IMPL_DEFER_NAMECAT_EXPANSION(l, r) \
	l##r

#define CEPO_IMPL_DEFER_NAMECAT(l, r) \
	CEPO_IMPL_DEFER_NAMECAT_EXPANSION(l, r)

#ifdef __COUNTER__
	#define CEPO_IMPL_DEFER_INDEX_MAKER(prefix) CEPO_IMPL_DEFER_NAMECAT(prefix, __COUNTER__)
#else
	#define CEPO_IMPL_DEFER_INDEX_MAKER(prefix) CEPO_IMPL_DEFER_NAMECAT(prefix, __LINE__)
#endif

#define CEPO_DEFER \
	const auto& CEPO_IMPL_DEFER_INDEX_MAKER(cepo_defer) \
	= ::cepo::impl::defer_maker() + [&]() noexcept -> void

#endif //CEPO_DEFER_HPP
