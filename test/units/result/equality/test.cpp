//operator== and operator!=: two results are equal when they are in the same
//state and the stored value or error compares equal; in the exceptional state
//the recorded exception is compared instead. The two operators do not have to
//be negations of each other, and neither do the ones of T and E.
#include "cepo/result.hpp"
#include "cepo/result/factory.hpp"

#include <cstdio>
#include <exception>
#include <stdexcept>
#include <string>
#include <utility>

namespace
{
	int g_fail = 0;

	void check(bool cond, const char* expr, int line)
	{
		if (!cond)
		{
			std::printf("FAIL line %d: %s\n", line, expr);
			++g_fail;
		}
	}
}

#define CHECK(expr) check((expr), #expr, __LINE__)

//Moving throws only when armed, so one type can produce an ok result and an
//exceptional one.
struct maybe_throws
{
	static bool armed;

	maybe_throws() = default;
	maybe_throws(maybe_throws&&)
	{
		if (armed) throw std::runtime_error("move boom");
	}

	//both operators are required even though this type can only ever produce
	//the exceptional state
	bool operator==(const maybe_throws&) const noexcept { return true; }
	bool operator!=(const maybe_throws&) const noexcept { return false; }
};

bool maybe_throws::armed = false;

struct throws_on_construction
{
	throws_on_construction() { throw std::runtime_error("ctor boom"); }

	bool operator==(const throws_on_construction&) const noexcept { return true; }
	bool operator!=(const throws_on_construction&) const noexcept { return false; }
};

//counts how often each operator is invoked
struct counted_both
{
	static int equality_calls;
	static int inequality_calls;

	int value;

	explicit counted_both(int v) noexcept: value(v) {}

	bool operator==(const counted_both& other) const noexcept
	{
		++equality_calls;
		return value == other.value;
	}

	bool operator!=(const counted_both& other) const noexcept
	{
		++inequality_calls;
		return value != other.value;
	}
};

int counted_both::equality_calls = 0;
int counted_both::inequality_calls = 0;

//the two operators are deliberately not negations of each other
struct perverse
{
	int value;

	explicit perverse(int v) noexcept: value(v) {}

	bool operator==(const perverse&) const noexcept { return true; }
	bool operator!=(const perverse&) const noexcept { return true; }
};

struct throwing_eq
{
	int value;

	bool operator==(const throwing_eq& other) const noexcept(false)
	{
		return value == other.value;
	}

	bool operator!=(const throwing_eq& other) const noexcept(false)
	{
		return value != other.value;
	}
};

struct throwing_inequality
{
	int value;

	bool operator==(const throwing_inequality& other) const noexcept
	{
		return value == other.value;
	}

	bool operator!=(const throwing_inequality& other) const noexcept(false)
	{
		return value != other.value;
	}
};

using plain = cepo::result<int, int>;
using armed = cepo::result<maybe_throws, int>;

auto main() -> int
{
	//ok vs ok
	{
		plain a = cepo::make_result_value<int>(1);
		plain b = cepo::make_result_value<int>(1);
		plain c = cepo::make_result_value<int>(2);

		CHECK(a == b);
		CHECK(!(a == c));
		CHECK(a != c);
		CHECK(!(a != b));

		CHECK(a == a);					//reflexive
		CHECK(!(a != a));
		CHECK((a == b) == (b == a));	//symmetric
		CHECK((a != b) == (b != a));
	}

	//error vs error
	{
		plain a = cepo::make_result_error<int>(9);
		plain b = cepo::make_result_error<int>(9);
		plain c = cepo::make_result_error<int>(10);

		CHECK(a == b);
		CHECK(!(a == c));
		CHECK(a != c);
		CHECK(!(a != b));
		CHECK((a == c) == (c == a));
		CHECK((a != c) == (c != a));
	}

	//different states: equal returns false, unequal returns true
	{
		plain ok = cepo::make_result_value<int>(0);
		plain error = cepo::make_result_error<int>(0);

		CHECK(!(ok == error));
		CHECK(!(error == ok));
		CHECK(ok != error);
		CHECK(error != ok);

		maybe_throws::armed = true;
		armed exceptional = cepo::make_result_value<maybe_throws>();
		armed error_armed = cepo::make_result_error<int>(0);
		CHECK(exceptional != error_armed);
		CHECK(error_armed != exceptional);

		maybe_throws::armed = false;
		armed ok_armed = cepo::make_result_value<maybe_throws>();
		CHECK(ok_armed.is_ok());
		CHECK(!(ok_armed == exceptional));
		CHECK(ok_armed != exceptional);
		CHECK(exceptional != ok_armed);
	}

	//exceptional vs exceptional compares the recorded exception
	{
		maybe_throws::armed = true;
		armed a = cepo::make_result_value<maybe_throws>();
		armed b = cepo::make_result_value<maybe_throws>();
		CHECK(!(a == b));		//two separate exceptions
		CHECK(a != b);

		armed shared(std::move(a));	//the move shares the exception
		CHECK(shared.is_exceptional());
		CHECK(a.is_exceptional());
		CHECK(shared == a);
		CHECK(a == shared);
		CHECK(!(shared != a));
		CHECK(!(a != shared));
	}

	//only the operator belonging to the comparison is used, and only when the
	//states match
	{
		counted_both::equality_calls = 0;
		counted_both::inequality_calls = 0;

		cepo::result<counted_both, int> a = cepo::make_result_value<counted_both>(1);
		cepo::result<counted_both, int> b = cepo::make_result_value<counted_both>(1);

		CHECK(a == b);
		CHECK(counted_both::equality_calls == 1);
		CHECK(counted_both::inequality_calls == 0);

		CHECK(!(a != b));
		CHECK(counted_both::equality_calls == 1);
		CHECK(counted_both::inequality_calls == 1);

		cepo::result<counted_both, int> e = cepo::make_result_error<int>(0);
		counted_both::equality_calls = 0;
		counted_both::inequality_calls = 0;

		CHECK(!(a == e));
		CHECK(a != e);
		CHECK(counted_both::equality_calls == 0);
		CHECK(counted_both::inequality_calls == 0);
	}

	//the operators of T are used as they are: no symmetry is invented
	{
		cepo::result<perverse, int> a = cepo::make_result_value<perverse>(1);
		cepo::result<perverse, int> b = cepo::make_result_value<perverse>(2);

		CHECK(a == b);		//perverse::operator== always says true
		CHECK(a != b);		//perverse::operator!= always says true
	}

	//const operands
	{
		const plain a = cepo::make_result_value<int>(3);
		const plain b = cepo::make_result_value<int>(3);
		const plain c = cepo::make_result_error<int>(3);
		CHECK(a == b);
		CHECK(!(a != b));
		CHECK(!(a == c));
		CHECK(a != c);
	}

	//a non-trivial payload
	{
		cepo::result<std::string, int> a = cepo::make_result_value<std::string>("cepo");
		cepo::result<std::string, int> b = cepo::make_result_value<std::string>("cepo");
		cepo::result<std::string, int> c = cepo::make_result_value<std::string>("cepo!");
		CHECK(a == b);
		CHECK(!(a != b));
		CHECK(!(a == c));
		CHECK(a != c);
	}

	//comparison does not modify its operands
	{
		plain a = cepo::make_result_value<int>(4);
		plain b = cepo::make_result_value<int>(4);
		CHECK(a == b);
		CHECK(!(a != b));
		CHECK(a.is_ok());
		CHECK(a.unwrap_value() == 4);
		CHECK(b.is_ok());
		CHECK(b.unwrap_value() == 4);
	}

	//void results carry no value, so two ok results are always equal
	{
		cepo::result<void, int> a = cepo::make_result_value<void>();
		cepo::result<void, int> b = cepo::make_result_value<void>();
		CHECK(a == b);
		CHECK(!(a != b));

		cepo::result<void, int> e1 = cepo::make_result_error<int>(1);
		cepo::result<void, int> e2 = cepo::make_result_error<int>(1);
		cepo::result<void, int> e3 = cepo::make_result_error<int>(2);
		CHECK(e1 == e2);
		CHECK(!(e1 != e2));
		CHECK(!(e1 == e3));
		CHECK(e1 != e3);
		CHECK(!(a == e1));
		CHECK(a != e1);
	}

	//the cv-qualified void specialisations behave the same way
	{
		cepo::result<const void, int> a = cepo::make_result_value<const void>();
		cepo::result<const void, int> b = cepo::make_result_value<const void>();
		CHECK(a == b);
		CHECK(!(a != b));

		cepo::result<volatile void, int> c = cepo::make_result_value<volatile void>();
		cepo::result<volatile void, int> d = cepo::make_result_value<volatile void>();
		CHECK(c == d);
		CHECK(!(c != d));

		cepo::result<const volatile void, int> e =
			cepo::make_result_value<const volatile void>();
		cepo::result<const volatile void, int> f =
			cepo::make_result_value<const volatile void>();
		CHECK(e == f);
		CHECK(!(e != f));
	}

	//void results share exceptions the same way
	{
		using void_exc = cepo::result<void, throws_on_construction>;
		void_exc a = cepo::make_result_error<throws_on_construction>();
		CHECK(a.is_exceptional());
		void_exc b(std::move(a));
		CHECK(b.is_exceptional());
		CHECK(a.is_exceptional());
		CHECK(a == b);
		CHECK(!(a != b));
	}

	//noexcept reflects the noexcept-ness of the compared types
	{
		plain a = cepo::make_result_value<int>(1);
		plain b = cepo::make_result_value<int>(1);
		static_assert(noexcept(a == b), "int equality is noexcept");
		static_assert(noexcept(a != b), "int inequality is noexcept");

		cepo::result<throwing_eq, int> c = cepo::make_result_value<throwing_eq>();
		cepo::result<throwing_eq, int> d = cepo::make_result_value<throwing_eq>();
		static_assert(!noexcept(c == d), "a throwing operator== must be visible");
		static_assert(!noexcept(c != d), "a throwing operator!= must be visible");

		cepo::result<throwing_inequality, int> e =
			cepo::make_result_value<throwing_inequality>();
		cepo::result<throwing_inequality, int> f =
			cepo::make_result_value<throwing_inequality>();
		static_assert(noexcept(e == f), "equality stays noexcept");
		static_assert(!noexcept(e != f), "inequality is not noexcept here");
	}

	std::printf("equality: %s\n", g_fail == 0 ? "ok" : "failed");
	return g_fail == 0 ? 0 : 1;
}
