#include "cepo/callable/is_callable.hpp"
#include "cepo/result.hpp"
#include "cepo/result/factory.hpp"
#include <cstdio>
#include <exception>
#include <stdexcept>
#include <string>
#include <utility>

static int g_pass = 0;
static int g_fail = 0;

static void check(bool ok, const char* what)
{
	if (ok) {
		++g_pass;
	} else {
		++g_fail;
		std::printf("  FAIL: %s\n", what);
	}
}

//Its move constructor throws, so make_result_value leaves the result in the
//exceptional state.
struct throwing_move
{
	throwing_move() = default;
	throwing_move(throwing_move&&) { throw std::runtime_error("move boom"); }
};

//Receives the exception pointer by value: the copy is made at the call.
struct exceptional_by_value
{
	std::string* out;
	void operator()(std::exception_ptr p) const
	{
		try {
			std::rethrow_exception(p);
		} catch (const std::runtime_error& e) {
			*out = e.what();
		}
	}
};

struct exceptional_by_const_ref
{
	std::string* out;
	void operator()(const std::exception_ptr& p) const
	{
		try {
			std::rethrow_exception(p);
		} catch (const std::runtime_error& e) {
			*out = e.what();
		}
	}
};

struct exceptional_by_rvalue_ref
{
	void operator()(std::exception_ptr&&) const {}
};

struct exceptional_by_ref
{
	void operator()(std::exception_ptr&) const {}
};

struct value_by_value
{
	int* out;
	void operator()(int v) const { *out = v; }
};

struct error_by_value
{
	int* out;
	void operator()(int e) const { *out = e; }
};

//The traits have to model the call that is actually made: the argument is a
//const std::exception_ptr&, so by-value and by-const-reference callbacks are
//supported and the other two are excluded.
static_assert(
	cepo::is_callable<exceptional_by_value, const std::exception_ptr&>::value,
	"by-value exception_ptr callback must be accepted");
static_assert(
	cepo::is_callable<exceptional_by_const_ref, const std::exception_ptr&>::value,
	"const& exception_ptr callback must be accepted");
static_assert(
	not cepo::is_callable<exceptional_by_rvalue_ref, const std::exception_ptr&>::value,
	"&& exception_ptr callback must be rejected");
static_assert(
	not cepo::is_callable<exceptional_by_ref, const std::exception_ptr&>::value,
	"non-const& exception_ptr callback must be rejected");

//Nothing about the value and error stages changed, but they follow the same
//rule, so check the by-value forms there as well.
static_assert(cepo::is_callable<value_by_value, int&>::value,
	"by-value T callback must be accepted");
static_assert(cepo::is_callable<value_by_value, const int&>::value,
	"by-value T callback must be accepted on a const result");
static_assert(cepo::is_callable<error_by_value, const int&>::value,
	"by-value E callback must be accepted");

static void exceptional_pointer_compared()
{
	std::string a;
	std::string b;

	cepo::result<throwing_move, int> r =
		cepo::make_result_value<throwing_move>();

	check(r.is_exceptional(), "result is exceptional");

	r.ok([](throwing_move&) {}).ignore_error()
		.exceptional(exceptional_by_value{&a});
	check(a == "move boom", "by-value exception_ptr receives the exception");

	cepo::result<throwing_move, int> s =
		cepo::make_result_value<throwing_move>();

	s.ok([](throwing_move&) {}).ignore_error()
		.exceptional(exceptional_by_const_ref{&b});
	check(b == "move boom", "const& exception_ptr receives the exception");
}

static void exceptional_is_skipped_for_other_states()
{
	bool called = false;

	cepo::result<int, int> ok = cepo::make_result_value<int>(1);
	ok.ok([](int&) {})
		.ignore_error()
		.exceptional([&](std::exception_ptr) { called = true; });
	check(not called, "by-value callback not called for an ok result");

	cepo::result<int, int> err = cepo::make_result_error<int>(2);
	err.ok([](int&) {})
		.error([](const int&) {})
		.exceptional([&](std::exception_ptr) { called = true; });
	check(not called, "by-value callback not called for an error result");
}

static void by_value_in_every_stage()
{
	int value = 0;
	cepo::result<int, int> r = cepo::make_result_value<int>(7);
	r.ok(value_by_value{&value}).ignore_error();
	check(value == 7, "ok passes T by value");

	int error = 0;
	cepo::result<int, int> e = cepo::make_result_error<int>(9);
	e.ok([](int&) {}).error(error_by_value{&error}).ignore_exceptional();
	check(error == 9, "error passes E by value");

	int const_value = 0;
	const cepo::result<int, int> c = cepo::make_result_value<int>(11);
	c.ok(value_by_value{&const_value}).ignore_error();
	check(const_value == 11, "const ok passes const T by value");
}

static void lvalue_functor_and_function_pointer()
{
	std::string message;

	cepo::result<throwing_move, int> r =
		cepo::make_result_value<throwing_move>();

	exceptional_by_value functor{&message};
	r.ok([](throwing_move&) {}).ignore_error().exceptional(functor);
	check(message == "move boom", "lvalue functor is bound and called");

	std::string from_pointer;
	cepo::result<throwing_move, int> p =
		cepo::make_result_value<throwing_move>();
	p.ok([](throwing_move&) {})
		.ignore_error()
		.exceptional(exceptional_by_value{&from_pointer});
	check(from_pointer == "move boom", "temporary functor is called");
}

auto main() -> int
{
	exceptional_pointer_compared();
	exceptional_is_skipped_for_other_states();
	by_value_in_every_stage();
	lvalue_functor_and_function_pointer();

	std::printf("passed %d, failed %d\n", g_pass, g_fail);
	return g_fail == 0 ? 0 : 1;
}
