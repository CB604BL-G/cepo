//The ok() / error() / ignore_error() chain: which callback runs depends on the
//state, and the chain must stay usable on const results and with different
//callback shapes.
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

struct maybe_throws
{
	static bool armed;

	maybe_throws() = default;
	maybe_throws(maybe_throws&&)
	{
		if (armed) throw std::runtime_error("move boom");
	}
};

bool maybe_throws::armed = false;

struct value_sink
{
	int* seen;
	void operator()(int& v) const { *seen = v; ++v; }
};

struct error_sink
{
	int* seen;
	void operator()(const int& v) const { *seen = v; }
};

void add_one(int& v) { ++v; }

auto main() -> int
{
	//ok result: the ok callback runs, the error callback does not
	{
		cepo::result<int, int> r = cepo::make_result_value<int>(10);
		bool ok_ran = false;
		bool error_ran = false;
		int error_value = 0;

		r.ok([&](int& v) { ok_ran = true; v += 1; })
			.error([&](const int& e) { error_ran = true; error_value = e; });

		CHECK(ok_ran);
		CHECK(!error_ran);
		CHECK(error_value == 0);
		CHECK(r.unwrap_value() == 11);		//the callback may modify the value
	}

	//error result: the ok callback does not run, the error callback does
	{
		cepo::result<int, int> r = cepo::make_result_error<int>(77);
		bool ok_ran = false;
		int error_value = 0;

		r.ok([&](int&) { ok_ran = true; })
			.error([&](const int& e) { error_value = e; });

		CHECK(!ok_ran);
		CHECK(error_value == 77);
	}

	//exceptional result: neither callback runs
	{
		maybe_throws::armed = true;
		cepo::result<maybe_throws, int> r =
			cepo::make_result_value<maybe_throws>();
		CHECK(r.is_exceptional());

		bool ok_ran = false;
		bool error_ran = false;

		r.ok([&](maybe_throws&) { ok_ran = true; })
			.error([&](const int&) { error_ran = true; });

		CHECK(!ok_ran);
		CHECK(!error_ran);
	}

	//ignore_error terminates the chain without touching the result
	{
		cepo::result<int, int> good = cepo::make_result_value<int>(1);
		good.ok([](int&) {}).ignore_error();
		CHECK(good.is_ok());
		CHECK(good.unwrap_value() == 1);

		cepo::result<int, int> bad = cepo::make_result_error<int>(2);
		bad.ok([](int&) {}).ignore_error();
		CHECK(bad.is_error());
		CHECK(bad.unwrap_error() == 2);
	}

	//the const overload of ok()
	{
		const cepo::result<int, int> good = cepo::make_result_value<int>(5);
		int seen = 0;
		good.ok([&](const int& v) { seen = v; }).ignore_error();
		CHECK(seen == 5);

		const cepo::result<int, int> bad = cepo::make_result_error<int>(6);
		int error_seen = 0;
		bad.ok([&](const int&) { CHECK(false); })
			.error([&](const int& e) { error_seen = e; });
		CHECK(error_seen == 6);
	}

	//callback shapes: an lvalue functor and a plain function pointer
	{
		cepo::result<int, int> r = cepo::make_result_value<int>(20);
		int seen = 0;
		value_sink sink{&seen};
		r.ok(sink).ignore_error();
		CHECK(seen == 20);
		CHECK(r.unwrap_value() == 21);		//the functor incremented it

		cepo::result<int, int> r2 = cepo::make_result_value<int>(30);
		r2.ok(&add_one).ignore_error();
		CHECK(r2.unwrap_value() == 31);

		cepo::result<int, int> err = cepo::make_result_error<int>(40);
		int error_seen = 0;
		error_sink err_sink{&error_seen};
		err.ok([](int&) {}).error(err_sink);
		CHECK(error_seen == 40);
	}

	//void results: the ok callback takes no argument
	{
		cepo::result<void, int> good = cepo::make_result_value<void>();
		bool ran = false;
		good.ok([&]() { ran = true; }).ignore_error();
		CHECK(ran);

		cepo::result<void, int> bad = cepo::make_result_error<int>(9);
		bool ran_on_error = false;
		int error_seen = 0;
		bad.ok([&]() { ran_on_error = true; })
			.error([&](const int& e) { error_seen = e; });
		CHECK(!ran_on_error);
		CHECK(error_seen == 9);
	}

	std::printf("ok_error_chain: %s\n", g_fail == 0 ? "ok" : "failed");
	return g_fail == 0 ? 0 : 1;
}
