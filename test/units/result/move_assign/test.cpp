//Move construction and move assignment between the three states, including
//self assignment and the documented behaviour of the source object.
#include "cepo/result.hpp"
#include "cepo/result/factory.hpp"

#include <cstdio>
#include <exception>
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

//Moving throws only when armed, so the same type can produce an ok result and
//an exceptional one.
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

using plain = cepo::result<int, int>;
using armed = cepo::result<maybe_throws, int>;

auto main() -> int
{
	//ok <- ok
	{
		plain a = cepo::make_result_value<int>(1);
		plain b = cepo::make_result_value<int>(2);
		a = std::move(b);
		CHECK(a.is_ok());
		CHECK(a.unwrap_value() == 2);
		CHECK(b.is_ok());		//the source keeps its state
	}

	//ok <- error
	{
		plain a = cepo::make_result_value<int>(1);
		plain b = cepo::make_result_error<int>(9);
		a = std::move(b);
		CHECK(a.is_error());
		CHECK(a.unwrap_error() == 9);
		CHECK(b.is_error());
	}

	//error <- ok
	{
		plain a = cepo::make_result_error<int>(1);
		plain b = cepo::make_result_value<int>(2);
		a = std::move(b);
		CHECK(a.is_ok());
		CHECK(a.unwrap_value() == 2);
		CHECK(b.is_ok());
	}

	//error <- error
	{
		plain a = cepo::make_result_error<int>(1);
		plain b = cepo::make_result_error<int>(2);
		a = std::move(b);
		CHECK(a.is_error());
		CHECK(a.unwrap_error() == 2);
		CHECK(b.is_error());
	}

	//ok <- exceptional: the exception is propagated and shared with the source
	{
		maybe_throws::armed = false;
		armed target = cepo::make_result_value<maybe_throws>();
		CHECK(target.is_ok());

		maybe_throws::armed = true;
		armed source = cepo::make_result_value<maybe_throws>();
		CHECK(source.is_exceptional());
		std::exception_ptr original = source.get_exception();

		target = std::move(source);
		CHECK(target.is_exceptional());
		CHECK(source.is_exceptional());
		CHECK(target.get_exception() == original);
		CHECK(source.get_exception() == original);
	}

	//exceptional <- ok: the exception is dropped
	{
		maybe_throws::armed = true;
		armed target = cepo::make_result_value<maybe_throws>();
		CHECK(target.is_exceptional());

		maybe_throws::armed = false;
		armed source = cepo::make_result_value<maybe_throws>();
		CHECK(source.is_ok());

		target = std::move(source);
		CHECK(target.is_ok());
		CHECK(!target.is_exceptional());
		CHECK(target.get_exception() == nullptr);
		CHECK(source.is_ok());
	}

	//exceptional <- exceptional: the assignment takes over the other exception
	//and the source keeps its own
	{
		maybe_throws::armed = true;
		armed a = cepo::make_result_value<maybe_throws>();
		armed b = cepo::make_result_value<maybe_throws>();
		std::exception_ptr a_original = a.get_exception();
		std::exception_ptr b_original = b.get_exception();
		CHECK(a_original != b_original);		//two distinct exceptions

		a = std::move(b);
		CHECK(a.is_exceptional());
		CHECK(b.is_exceptional());
		CHECK(a.get_exception() == b_original);	//the target now shares b's
		CHECK(b.get_exception() == b_original);	//the source still holds it
		CHECK(a.get_exception() != a_original);	//its own exception is gone
	}

	//self move assignment does nothing
	{
		plain a = cepo::make_result_value<int>(5);
		plain& alias = a;
		a = std::move(alias);
		CHECK(a.is_ok());
		CHECK(a.unwrap_value() == 5);

		maybe_throws::armed = true;
		armed e = cepo::make_result_value<maybe_throws>();
		armed& e_alias = e;
		std::exception_ptr before = e.get_exception();
		e = std::move(e_alias);
		CHECK(e.is_exceptional());
		CHECK(e.get_exception() == before);
	}

	//move construction keeps the state of both sides
	{
		plain a = cepo::make_result_value<int>(6);
		plain b(std::move(a));
		CHECK(b.is_ok());
		CHECK(b.unwrap_value() == 6);
		CHECK(a.is_ok());

		plain c = cepo::make_result_error<int>(7);
		plain d(std::move(c));
		CHECK(d.is_error());
		CHECK(d.unwrap_error() == 7);
		CHECK(c.is_error());
	}

	//void results follow the same rules
	{
		cepo::result<void, int> a = cepo::make_result_value<void>();
		cepo::result<void, int> b(std::move(a));
		CHECK(a.is_ok());
		CHECK(b.is_ok());

		cepo::result<void, int> e = cepo::make_result_error<int>(8);
		CHECK(e.is_error());
		CHECK(e.unwrap_error() == 8);
		e = std::move(b);
		CHECK(e.is_ok());
		CHECK(b.is_ok());
	}

	std::printf("move_assign: %s\n", g_fail == 0 ? "ok" : "failed");
	return g_fail == 0 ? 0 : 1;
}
