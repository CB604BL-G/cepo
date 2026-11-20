//operator!=: the negation of operator== for the types that define their
//operators as negations of each other, and an independent delegation for the
//types that do not. The two cases are both checked here, because the header
//documents that cepo::result follows T, E and std::exception_ptr rather than
//negating operator== itself.
#include "cepo/result.hpp"
#include "cepo/result/factory.hpp"

#include <cstdio>
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

//operator== and operator!= that are not negations of each other.
struct asymmetric
{
	int value;

	explicit asymmetric(int v) noexcept: value(v) {}

	bool operator==(const asymmetric&) const noexcept { return true; }
	bool operator!=(const asymmetric&) const noexcept { return true; }
};

struct asymmetric_negated
{
	int value;

	explicit asymmetric_negated(int v) noexcept: value(v) {}

	bool operator==(const asymmetric_negated&) const noexcept { return false; }
	bool operator!=(const asymmetric_negated&) const noexcept { return false; }
};

//not noexcept, to check that the qualification is propagated.
struct throwing_ne
{
	int value;

	explicit throwing_ne(int v) noexcept: value(v) {}

	bool operator==(const throwing_ne& o) const noexcept { return value == o.value; }
	bool operator!=(const throwing_ne& o) const noexcept(false) { return value != o.value; }
};

//One type that can produce both states, so the exceptional one is reachable.
struct maybe_throws
{
	static bool armed;

	maybe_throws() = default;

	maybe_throws(maybe_throws&&)
	{
		if (armed) throw std::runtime_error("move boom");
	}

	bool operator==(const maybe_throws&) const noexcept { return true; }
	bool operator!=(const maybe_throws&) const noexcept { return false; }
};

bool maybe_throws::armed = false;

using plain = cepo::result<int, int>;

auto main() -> int
{
	//ok vs ok
	{
		plain a = cepo::make_result_value<int>(1);
		plain b = cepo::make_result_value<int>(2);
		plain same = cepo::make_result_value<int>(1);

		CHECK(a != b);
		CHECK(!(a != same));
		CHECK(!(a != a));						//irreflexive
		CHECK((a != b) == (b != a));			//symmetric
		CHECK((a == b) == !(a != b));			//negation of ==
	}

	//error vs error
	{
		plain a = cepo::make_result_error<int>(3);
		plain b = cepo::make_result_error<int>(4);
		plain same = cepo::make_result_error<int>(3);

		CHECK(a != b);
		CHECK(!(a != same));
		CHECK((a == b) == !(a != b));
	}

	//different states are never equal, so the comparison is true either way
	{
		plain ok = cepo::make_result_value<int>(0);
		plain error = cepo::make_result_error<int>(0);
		CHECK(ok != error);
		CHECK(error != ok);
	}

	//const operands
	{
		const plain a = cepo::make_result_value<int>(5);
		const plain b = cepo::make_result_value<int>(6);
		const plain same = cepo::make_result_value<int>(5);
		CHECK(a != b);
		CHECK(!(a != same));
	}

	//a non-trivial payload
	{
		cepo::result<std::string, int> a = cepo::make_result_value<std::string>("cepo");
		cepo::result<std::string, int> b = cepo::make_result_value<std::string>("cepo!");
		CHECK(a != b);
		CHECK(!(a != a));
	}

	//the exceptional state compares the recorded exception
	{
		using armed = cepo::result<maybe_throws, int>;

		maybe_throws::armed = true;
		armed a = cepo::make_result_value<maybe_throws>();
		armed b = cepo::make_result_value<maybe_throws>();
		CHECK(a != b);					//two separate exceptions

		armed shared(std::move(a));		//the move shares the exception
		CHECK(shared.is_exceptional());
		CHECK(!(shared != a));
		CHECK(!(a != shared));

		maybe_throws::armed = false;
		armed ok = cepo::make_result_value<maybe_throws>();
		CHECK(ok != b);
		CHECK(b != ok);
	}

	//when T defines == and != as something other than negations of each other,
	//cepo::result follows T instead of negating its own operator==
	{
		cepo::result<asymmetric, int> a = cepo::make_result_value<asymmetric>(0);
		cepo::result<asymmetric, int> b = cepo::make_result_value<asymmetric>(0);
		CHECK(a == b);
		CHECK(a != b);					//both true, as T says

		cepo::result<asymmetric_negated, int> c =
			cepo::make_result_value<asymmetric_negated>(0);
		cepo::result<asymmetric_negated, int> d =
			cepo::make_result_value<asymmetric_negated>(0);
		CHECK(!(c == d));
		CHECK(!(c != d));				//both false, as T says
	}

	//the qualification follows the compared operator
	{
		plain a = cepo::make_result_value<int>(1);
		plain b = cepo::make_result_value<int>(1);
		static_assert(noexcept(a != b), "int comparison is noexcept");

		cepo::result<throwing_ne, int> c = cepo::make_result_value<throwing_ne>(0);
		cepo::result<throwing_ne, int> d = cepo::make_result_value<throwing_ne>(0);
		static_assert(!noexcept(c != d), "a throwing operator!= must be visible");
	}

	//void results carry no value, so two ok results are never different
	{
		cepo::result<void, int> a = cepo::make_result_value<void>();
		cepo::result<void, int> b = cepo::make_result_value<void>();
		CHECK(!(a != b));

		cepo::result<void, int> e1 = cepo::make_result_error<int>(8);
		cepo::result<void, int> e2 = cepo::make_result_error<int>(8);
		cepo::result<void, int> e3 = cepo::make_result_error<int>(9);
		CHECK(!(e1 != e2));
		CHECK(e1 != e3);
		CHECK(a != e1);
	}

	//the cv-qualified void specialisations behave the same way
	{
		cepo::result<const void, int> a = cepo::make_result_value<const void>();
		cepo::result<const void, int> b = cepo::make_result_value<const void>();
		cepo::result<const void, int> e = cepo::make_result_error<int>(10);
		CHECK(!(a != b));
		CHECK(a != e);

		cepo::result<volatile void, int> c = cepo::make_result_value<volatile void>();
		cepo::result<volatile void, int> d = cepo::make_result_value<volatile void>();
		CHECK(!(c != d));

		cepo::result<const volatile void, int> f =
			cepo::make_result_value<const volatile void>();
		cepo::result<const volatile void, int> g =
			cepo::make_result_value<const volatile void>();
		CHECK(!(f != g));
	}

	//comparison does not modify its operands
	{
		plain a = cepo::make_result_value<int>(7);
		plain b = cepo::make_result_error<int>(7);
		CHECK(a != b);
		CHECK(a.is_ok());
		CHECK(a.unwrap_value() == 7);
		CHECK(b.is_error());
		CHECK(b.unwrap_error() == 7);
	}

	std::printf("inequality: %s\n", g_fail == 0 ? "ok" : "failed");
	return g_fail == 0 ? 0 : 1;
}
