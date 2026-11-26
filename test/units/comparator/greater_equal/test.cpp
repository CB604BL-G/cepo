//Contract of cepo::comparator::greater_equal, the ordering predicate the
//library offers so callers stop writing their own functor.
//
//Three properties are checked. The call must agree with the built-in
//operator, for builtin operands and for a user type that supplies only
//operator>=. And the functor must plug into the traits the library composes
//with, cepo::is_callable and cepo::is_nothrow_callable:
//
//  callable  <=> "l >= r" is well formed AND its result is implicitly
//                bool-testable, because the return type gate requires exactly
//                that conversion;
//  nothrow   <=> that bool conversion is nothrow AND the comparison itself is
//                nothrow, so a throwing operator>= makes the comparator report
//                "callable but throwing" instead of claiming a no-throw call;
//  constexpr <=> the call is usable in a constant expression, which needs the
//                operands' own operator to be constexpr as well.
#include "cepo/callable/is_callable.hpp"
#include "cepo/callable/is_nothrow_callable.hpp"
#include "cepo/comparator/greater_equal.hpp"

#include <cstdio>

namespace
{
	int g_fail = 0;

	void check(bool cond, const char* expr, int line)
	{
		if (!cond)
		{
			std::printf("  FAIL: %s (line %d)\n", expr, line);
			++g_fail;
		}
	}
}

#define CHECK(expr) check(static_cast<bool>(expr), #expr, __LINE__)

//A comparison result may be any type with an implicit bool conversion.
struct truthy
{
	bool v;

	operator bool() const noexcept
	{
		return v;
	}
};

//Explicit conversion only: is_bool_testable rejects it, so the comparator has
//to refuse the call rather than accept a result it cannot return as bool.
struct explicit_truthy
{
	bool v;

	explicit operator bool() const noexcept
	{
		return v;
	}
};

//Implicit but throwing conversion: usable, just not nothrow.
struct throwing_truthy
{
	bool v;

	operator bool() const
	{
		return v;
	}
};

struct sample
{
	int v;
};

bool operator>=(sample const& l, sample const& r) noexcept
{
	return l.v >= r.v;
}

struct throwing_sample
{
	int v;
};

bool operator>=(throwing_sample const& l, throwing_sample const& r)
{
	return l.v >= r.v;
}

struct wrapped_sample
{
	int v;
};

truthy operator>=(wrapped_sample const& l, wrapped_sample const& r) noexcept
{
	return truthy{l.v >= r.v};
}

struct conversion_sample
{
	int v;
};

throwing_truthy operator>=(conversion_sample const& l, conversion_sample const& r) noexcept
{
	return throwing_truthy{l.v >= r.v};
}

struct explicit_sample
{
	int v;
};

explicit_truthy operator>=(explicit_sample const& l, explicit_sample const& r) noexcept
{
	return explicit_truthy{l.v >= r.v};
}

struct void_sample
{
};

void operator>=(void_sample const&, void_sample const&)
{
}

struct unrelated
{
};

//A user type joins a constant expression as soon as its own operator is
//constexpr; nothing in the comparator prevents that.
struct literal_sample
{
	int v;
};

constexpr bool operator>=(literal_sample const& l, literal_sample const& r) noexcept
{
	return l.v >= r.v;
}

//constexpr is part of the contract. If operator() ever loses it, these
//assertions stop compiling instead of the item quietly passing.
static_assert(cepo::comparator::greater_equal{}(2, 2), "constexpr: greater_equal{}(2, 2)");
static_assert(!cepo::comparator::greater_equal{}(1, 2), "constexpr: !greater_equal{}(1, 2)");
static_assert(cepo::comparator::greater_equal{}(literal_sample{2}, literal_sample{2}),
	"constexpr: user-defined operands");

int main()
{
	const cepo::comparator::greater_equal ge;

	//The call agrees with the built-in operator.
	CHECK(ge(2, 2));
	CHECK(ge(2, 1));
	CHECK(!ge(1, 2));
	CHECK(ge(0, -1));
	CHECK(ge(2L, 2));
	CHECK(!ge(1L, 2));

	//A user type needs nothing but operator>=.
	sample low{1};
	sample high{2};
	CHECK(ge(high, low));
	CHECK(ge(low, low));
	CHECK(!ge(low, high));

	//A bool-like result is accepted, not just a literal bool.
	CHECK(ge(wrapped_sample{2}, wrapped_sample{2}));
	CHECK(ge(wrapped_sample{2}, wrapped_sample{1}));
	CHECK(!ge(wrapped_sample{1}, wrapped_sample{2}));

	//Builtin operands: available and nothrow.
	CHECK((cepo::is_callable<cepo::comparator::greater_equal, int, int>::value));
	CHECK((cepo::is_nothrow_callable<cepo::comparator::greater_equal, int, int>::value));
	CHECK((cepo::is_callable<cepo::comparator::greater_equal, int, long>::value));
	CHECK((cepo::is_nothrow_callable<cepo::comparator::greater_equal, int, long>::value));

	//A const comparator is what an algorithm or a container hands out.
	CHECK((cepo::is_callable<const cepo::comparator::greater_equal&, int, int>::value));
	CHECK((cepo::is_nothrow_callable<const cepo::comparator::greater_equal&, int, int>::value));

	//User-defined operands.
	CHECK((cepo::is_callable<cepo::comparator::greater_equal, sample, sample>::value));
	CHECK((cepo::is_nothrow_callable<cepo::comparator::greater_equal, sample, sample>::value));
	CHECK((cepo::is_callable<cepo::comparator::greater_equal, wrapped_sample, wrapped_sample>::value));
	CHECK((cepo::is_nothrow_callable<cepo::comparator::greater_equal, wrapped_sample, wrapped_sample>::value));

	//A throwing operator>= stays callable but stops being nothrow.
	CHECK((cepo::is_callable<cepo::comparator::greater_equal, throwing_sample, throwing_sample>::value));
	CHECK((!cepo::is_nothrow_callable<cepo::comparator::greater_equal, throwing_sample, throwing_sample>::value));

	//A throwing bool conversion has the same effect.
	CHECK((cepo::is_callable<cepo::comparator::greater_equal, conversion_sample, conversion_sample>::value));
	CHECK((!cepo::is_nothrow_callable<cepo::comparator::greater_equal, conversion_sample, conversion_sample>::value));

	//A result that is not implicitly bool-testable is refused by the return
	//type gate, so the trait reports "not callable" rather than "throwing".
	CHECK((!cepo::is_callable<cepo::comparator::greater_equal, explicit_sample, explicit_sample>::value));
	CHECK((!cepo::is_nothrow_callable<cepo::comparator::greater_equal, explicit_sample, explicit_sample>::value));
	CHECK((!cepo::is_callable<cepo::comparator::greater_equal, void_sample, void_sample>::value));

	//No operator at all: same refusal.
	CHECK((!cepo::is_callable<cepo::comparator::greater_equal, unrelated, unrelated>::value));
	CHECK((!cepo::is_nothrow_callable<cepo::comparator::greater_equal, unrelated, unrelated>::value));

	std::printf("greater_equal: %s\n", g_fail == 0 ? "ok" : "failed");
	return g_fail == 0 ? 0 : 1;
}
