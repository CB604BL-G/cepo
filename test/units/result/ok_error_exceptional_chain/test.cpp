//The full chain: ok() -> error() -> exceptional(). Each stage forwards the
//state the previous one saw, so which callbacks run is decided by the state of
//the result alone: an ok result reaches the ok callback only, an error result
//the error callback only, and an exceptional result the exceptional callback
//only. ignore_ok() and ignore_error() cut a stage short without changing what
//the remaining stages see.
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

//Moving throws only when armed, so one type can produce both an ok result and
//an exceptional one.
struct maybe_throws
{
	static bool armed;

	maybe_throws() = default;
	maybe_throws(maybe_throws&&)
	{
		if (armed) throw std::runtime_error("value move boom");
	}
};

bool maybe_throws::armed = false;

//The slot -> result transfer moves the stored value, so an error type whose
//move constructor throws makes make_result_error fail as well.
struct error_throws_on_move
{
	static bool armed;

	int value;

	explicit error_throws_on_move(int v) noexcept: value(v) {}

	error_throws_on_move(error_throws_on_move&& other): value(other.value)
	{
		if (armed) throw std::runtime_error("error move boom");
	}
};

bool error_throws_on_move::armed = false;

//Records which stage ran, so "exactly once" can be checked per state.
struct counter
{
	int values = 0;
	int errors = 0;
	int exceptions = 0;
};

auto main() -> int
{
	//ok result: the ok callback runs and nothing else
	{
		cepo::result<int, int> r = cepo::make_result_value<int>(10);
		counter c;

		r.ok([&](int& v) { ++c.values; v += 1; })
			.error([&](const int&) { ++c.errors; })
			.exceptional([&](std::exception_ptr) { ++c.exceptions; });

		CHECK(c.values == 1);
		CHECK(c.errors == 0);
		CHECK(c.exceptions == 0);
		CHECK(r.unwrap_value() == 11);		//the callback may modify the value
	}

	//error result: the ok callback is skipped, the error callback runs
	{
		cepo::result<int, int> r = cepo::make_result_error<int>(77);
		counter c;
		int error_value = 0;

		r.ok([&](int&) { ++c.values; })
			.error([&](const int& e) { ++c.errors; error_value = e; })
			.exceptional([&](std::exception_ptr) { ++c.exceptions; });

		CHECK(c.values == 0);
		CHECK(c.errors == 1);
		CHECK(c.exceptions == 0);
		CHECK(error_value == 77);
	}

	//exceptional result: only the exceptional callback runs, and it receives
	//the recorded exception
	{
		maybe_throws::armed = true;
		cepo::result<maybe_throws, int> r =
			cepo::make_result_value<maybe_throws>();
		maybe_throws::armed = false;

		CHECK(r.is_exceptional());

		counter c;
		std::string message;

		r.ok([&](maybe_throws&) { ++c.values; })
			.error([&](const int&) { ++c.errors; })
			.exceptional([&](std::exception_ptr e)
			{
				++c.exceptions;
				try { std::rethrow_exception(e); }
				catch (const std::runtime_error& ex) { message = ex.what(); }
			});

		CHECK(c.values == 0);
		CHECK(c.errors == 0);
		CHECK(c.exceptions == 1);
		CHECK(message == "value move boom");
	}

	//an error that fails to move leaves the result exceptional, so the error
	//callback must not run even though make_result_error was called
	{
		error_throws_on_move::armed = true;
		cepo::result<int, error_throws_on_move> r =
			cepo::make_result_error<error_throws_on_move>(5);
		error_throws_on_move::armed = false;

		CHECK(r.is_exceptional());

		counter c;
		std::string message;

		r.ok([&](int&) { ++c.values; })
			.error([&](const error_throws_on_move&) { ++c.errors; })
			.exceptional([&](const std::exception_ptr& e)
			{
				++c.exceptions;
				try { std::rethrow_exception(e); }
				catch (const std::runtime_error& ex) { message = ex.what(); }
			});

		CHECK(c.values == 0);
		CHECK(c.errors == 0);
		CHECK(c.exceptions == 1);
		CHECK(message == "error move boom");
	}

	//ignore_error() cuts the first stage short but still forwards the exception
	{
		maybe_throws::armed = true;
		cepo::result<maybe_throws, int> r =
			cepo::make_result_value<maybe_throws>();
		maybe_throws::armed = false;

		bool exceptional_ran = false;
		r.ok([&](maybe_throws&) { CHECK(false); })
			.ignore_error()
			.exceptional([&](std::exception_ptr) { exceptional_ran = true; });
		CHECK(exceptional_ran);
	}

	//ignore_error() on an ok or an error result forwards nothing
	{
		cepo::result<int, int> good = cepo::make_result_value<int>(3);
		bool ran = false;
		good.ok([](int&) {}).ignore_error()
			.exceptional([&](std::exception_ptr) { ran = true; });
		CHECK(!ran);
		CHECK(good.is_ok());
		CHECK(good.unwrap_value() == 3);

		cepo::result<int, int> bad = cepo::make_result_error<int>(4);
		bad.ok([](int&) {}).ignore_error()
			.exceptional([&](std::exception_ptr) { ran = true; });
		CHECK(!ran);
		CHECK(bad.is_error());
		CHECK(bad.unwrap_error() == 4);
	}

	//ignore_ok() skips the value callback and keeps the rest of the chain
	{
		cepo::result<int, int> good = cepo::make_result_value<int>(5);
		bool error_ran = false;
		good.ignore_ok()
			.error([&](const int&) { error_ran = true; })
			.exceptional([](std::exception_ptr) { CHECK(false); });
		CHECK(!error_ran);
		CHECK(good.unwrap_value() == 5);		//the value is not touched

		cepo::result<int, int> bad = cepo::make_result_error<int>(6);
		int error_value = 0;
		bad.ignore_ok()
			.error([&](const int& e) { error_value = e; })
			.exceptional([](std::exception_ptr) { CHECK(false); });
		CHECK(error_value == 6);
	}

	//both stages ignored: the chain is consumed without running any callback
	{
		cepo::result<int, int> good = cepo::make_result_value<int>(7);
		good.ignore_ok().ignore_error().ignore_exceptional();
		CHECK(good.is_ok());
		CHECK(good.unwrap_value() == 7);

		cepo::result<int, int> bad = cepo::make_result_error<int>(8);
		bad.ignore_ok().ignore_error().ignore_exceptional();
		CHECK(bad.is_error());
		CHECK(bad.unwrap_error() == 8);

		maybe_throws::armed = true;
		cepo::result<maybe_throws, int> exceptional =
			cepo::make_result_value<maybe_throws>();
		maybe_throws::armed = false;
		exceptional.ok([&](maybe_throws&) { CHECK(false); })
			.ignore_error()
			.ignore_exceptional();
		CHECK(exceptional.is_exceptional());
	}

	//the const overloads follow the same rules
	{
		const cepo::result<int, int> good = cepo::make_result_value<int>(9);
		int seen = 0;
		good.ok([&](const int& v) { seen = v; })
			.error([](const int&) { CHECK(false); })
			.exceptional([](std::exception_ptr) { CHECK(false); });
		CHECK(seen == 9);

		const cepo::result<int, int> bad = cepo::make_result_error<int>(11);
		int error_seen = 0;
		bad.ignore_ok()
			.error([&](const int& e) { error_seen = e; })
			.exceptional([](std::exception_ptr) { CHECK(false); });
		CHECK(error_seen == 11);
	}

	//void results: the ok callback takes no argument, the rest is unchanged
	{
		cepo::result<void, int> good = cepo::make_result_value<void>();
		bool ok_ran = false;
		good.ok([&]() { ok_ran = true; })
			.error([](const int&) { CHECK(false); })
			.exceptional([](std::exception_ptr) { CHECK(false); });
		CHECK(ok_ran);

		cepo::result<void, int> bad = cepo::make_result_error<int>(12);
		bool ok_ran_on_error = false;
		int error_seen = 0;
		bad.ok([&]() { ok_ran_on_error = true; })
			.error([&](const int& e) { error_seen = e; })
			.exceptional([](std::exception_ptr) { CHECK(false); });
		CHECK(!ok_ran_on_error);
		CHECK(error_seen == 12);

		cepo::result<void, int> quiet = cepo::make_result_value<void>();
		quiet.ignore_ok().ignore_error().ignore_exceptional();
		CHECK(quiet.is_ok());
	}

	std::printf("ok_error_exceptional_chain: %s\n", g_fail == 0 ? "ok" : "failed");
	return g_fail == 0 ? 0 : 1;
}
