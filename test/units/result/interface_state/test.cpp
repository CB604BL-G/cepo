//State queries and the failures of unwrap: the reported reason and what() text
//must match the state that was actually violated.
#include "cepo/result.hpp"
#include "cepo/result/bad_result_unwrap.hpp"
#include "cepo/result/factory.hpp"

#include <cstdio>
#include <cstring>
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

struct throws_on_construction
{
	throws_on_construction() { throw std::runtime_error("original message"); }
};

using ok_result = cepo::result<int, int>;
using exc_result = cepo::result<throws_on_construction, int>;

auto main() -> int
{
	ok_result good = cepo::make_result_value<int>(42);
	ok_result bad = cepo::make_result_error<int>(7);
	exc_result broken = cepo::make_result_value<throws_on_construction>();

	//state queries
	CHECK(good.is_ok());
	CHECK(!good.is_error());
	CHECK(!good.is_exceptional());

	CHECK(!bad.is_ok());
	CHECK(bad.is_error());
	CHECK(!bad.is_exceptional());

	CHECK(!broken.is_ok());
	CHECK(!broken.is_error());
	CHECK(broken.is_exceptional());

	//the matching accessor succeeds and returns what was stored
	CHECK(good.unwrap_value() == 42);
	CHECK(bad.unwrap_error() == 7);

	//get_exception is null unless the state is exceptional
	CHECK(good.get_exception() == nullptr);
	CHECK(bad.get_exception() == nullptr);
	CHECK(broken.get_exception() != nullptr);

	//the recorded exception can be rethrown as the original
	{
		std::string message;
		try { std::rethrow_exception(broken.get_exception()); }
		catch (const std::exception& e) { message = e.what(); }
		CHECK(message == "original message");
	}

	//unwrap_value on an error result
	{
		bool thrown = false;
		try { (void)bad.unwrap_value(); }
		catch (const cepo::bad_result_unwrap& e)
		{
			thrown = true;
			CHECK(e.reason() == cepo::bad_result_unwrap_reason::unwrap_value_in_error_state);
			CHECK(std::strstr(e.what(), "unwrap_value() called on error result") != nullptr);
		}
		CHECK(thrown);
	}

	//unwrap_error on an ok result
	{
		bool thrown = false;
		try { (void)good.unwrap_error(); }
		catch (const cepo::bad_result_unwrap& e)
		{
			thrown = true;
			CHECK(e.reason() == cepo::bad_result_unwrap_reason::unwrap_error_in_ok_state);
			CHECK(std::strstr(e.what(), "unwrap_error() called on ok result") != nullptr);
		}
		CHECK(thrown);
	}

	//both unwraps on an exceptional result
	{
		bool value_thrown = false;
		try { (void)broken.unwrap_value(); }
		catch (const cepo::bad_result_unwrap& e)
		{
			value_thrown = true;
			CHECK(e.reason() == cepo::bad_result_unwrap_reason::unwrap_in_exceptional_state);
			CHECK(std::strstr(e.what(), "unwrap_*() called on exceptional result") != nullptr);
		}
		CHECK(value_thrown);

		bool error_thrown = false;
		try { (void)broken.unwrap_error(); }
		catch (const cepo::bad_result_unwrap& e)
		{
			error_thrown = true;
			CHECK(e.reason() == cepo::bad_result_unwrap_reason::unwrap_in_exceptional_state);
		}
		CHECK(error_thrown);
	}

	//the const overloads agree with the non-const ones
	{
		const ok_result& const_good = good;
		CHECK(const_good.is_ok());
		CHECK(const_good.unwrap_value() == 42);

		const ok_result& const_bad = bad;
		bool thrown = false;
		try { (void)const_bad.unwrap_value(); }
		catch (const cepo::bad_result_unwrap& e)
		{
			thrown = true;
			CHECK(e.reason() == cepo::bad_result_unwrap_reason::unwrap_value_in_error_state);
		}
		CHECK(thrown);
	}

	//the exception object itself: copy, move and assignment keep the reason
	{
		cepo::bad_result_unwrap needed_wrap(
			cepo::bad_result_unwrap_reason::unwrap_in_exceptional_state);

		cepo::bad_result_unwrap copied(needed_wrap);
		CHECK(copied.reason() == needed_wrap.reason());

		cepo::bad_result_unwrap moved(std::move(copied));
		CHECK(moved.reason() == needed_wrap.reason());

		cepo::bad_result_unwrap other(
			cepo::bad_result_unwrap_reason::unwrap_error_in_ok_state);
		other = moved;
		CHECK(other.reason() == needed_wrap.reason());

		CHECK(std::strcmp(needed_wrap.what(), other.what()) == 0);
	}

	//the three reasons are distinguishable through what()
	{
		cepo::bad_result_unwrap in_ok(cepo::bad_result_unwrap_reason::unwrap_error_in_ok_state);
		cepo::bad_result_unwrap in_error(cepo::bad_result_unwrap_reason::unwrap_value_in_error_state);
		cepo::bad_result_unwrap in_exceptional(
			cepo::bad_result_unwrap_reason::unwrap_in_exceptional_state);

		CHECK(std::strcmp(in_ok.what(), in_error.what()) != 0);
		CHECK(std::strcmp(in_ok.what(), in_exceptional.what()) != 0);
		CHECK(std::strcmp(in_error.what(), in_exceptional.what()) != 0);
	}

	std::printf("interface_state: %s\n", g_fail == 0 ? "ok" : "failed");
	return g_fail == 0 ? 0 : 1;
}
