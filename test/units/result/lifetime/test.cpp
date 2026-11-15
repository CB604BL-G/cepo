//Construction and destruction. A result owns exactly one value or one error,
//or neither when the state is exceptional; moving must not leak or destroy
//twice.
#include "cepo/result.hpp"
#include "cepo/result/factory.hpp"

#include <cstdio>
#include <stdexcept>
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

struct counted
{
	static int alive;
	static int moved;

	int value;

	explicit counted(int v) noexcept: value(v) { ++alive; }
	counted(const counted& other): value(other.value) { ++alive; }
	counted(counted&& other) noexcept: value(other.value) { other.value = -1; ++alive; ++moved; }
	counted& operator=(const counted&) = delete;
	counted& operator=(counted&& other) noexcept
	{
		value = other.value;
		other.value = -1;
		++moved;
		return *this;
	}
	~counted() noexcept { --alive; }
};

int counted::alive = 0;
int counted::moved = 0;

struct throws_on_construction
{
	throws_on_construction() { throw std::runtime_error("ctor boom"); }
};

struct throws_on_move
{
	throws_on_move() = default;
	throws_on_move(throws_on_move&&) { throw std::runtime_error("move boom"); }
};

auto main() -> int
{
	//the ok state owns exactly one value
	{
		cepo::result<counted, int> r = cepo::make_result_value<counted>(1);
		CHECK(r.is_ok());
		CHECK(counted::alive == 1);
	}
	CHECK(counted::alive == 0);

	//the error state owns exactly one error
	{
		cepo::result<int, counted> r = cepo::make_result_error<counted>(2);
		CHECK(r.is_error());
		CHECK(counted::alive == 1);
	}
	CHECK(counted::alive == 0);

	//constructing the stored value fails: nothing is left alive
	{
		cepo::result<throws_on_construction, int> r =
			cepo::make_result_value<throws_on_construction>();
		CHECK(r.is_exceptional());
		CHECK(!r.is_ok());
		CHECK(!r.is_error());
		CHECK(r.get_exception() != nullptr);
	}

	//the move from the slot into the result fails: same outcome
	{
		cepo::result<throws_on_move, int> r =
			cepo::make_result_value<throws_on_move>();
		CHECK(r.is_exceptional());
		CHECK(r.get_exception() != nullptr);
	}

	//overwriting a value with an error destroys the value
	{
		cepo::result<counted, int> a = cepo::make_result_value<counted>(3);
		CHECK(counted::alive == 1);

		cepo::result<counted, int> b = cepo::make_result_error<int>(4);
		a = std::move(b);
		CHECK(a.is_error());
		CHECK(counted::alive == 0);
	}

	//overwriting an error with a value destroys the error
	{
		cepo::result<int, counted> a = cepo::make_result_error<counted>(5);
		cepo::result<int, counted> b = cepo::make_result_value<int>(6);
		a = std::move(b);
		CHECK(a.is_ok());
		CHECK(counted::alive == 0);
	}

	//moving a value into a new result leaves exactly one object alive per side
	{
		cepo::result<counted, int> a = cepo::make_result_value<counted>(7);
		cepo::result<counted, int> b(std::move(a));
		CHECK(b.is_ok());
		CHECK(counted::alive == 2);
	}
	CHECK(counted::alive == 0);

	//the void specialisation owns no value at all
	{
		cepo::result<void, int> r = cepo::make_result_value<void>();
		CHECK(r.is_ok());
		CHECK(!r.is_exceptional());
	}

	CHECK(counted::alive == 0);

	std::printf("lifetime: %s\n", g_fail == 0 ? "ok" : "failed");
	return g_fail == 0 ? 0 : 1;
}
