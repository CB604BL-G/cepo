//Contract of cepo::function_source_location: the file name, the function name
//and the line are read at the call site, and the function name comes from the
//compiler helper that the header picks for the current compiler.
//
//The exact text of that helper differs between compilers, so every assertion
//below looks for a substring of a name this file controls. The shape produced
//for a lambda is compiler-specific and is only checked for being non-empty.
#include "cepo/source_location.hpp"

#include <cstdio>
#include <cstring>
#include <type_traits>

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

using functional = cepo::function_source_location;

//Compile-time part: all three accessors are constexpr.
constexpr functional compile_time{__FILE__, "a_recorded_name", 7u};
static_assert(compile_time.line() == 7u, "the line is stored verbatim");
static_assert(compile_time.function_name() != nullptr, "the function name is not null");
static_assert(compile_time.function_name()[0] == 'a', "the function name is stored verbatim");
static_assert(std::is_same<decltype(compile_time.line()), unsigned>::value, "line() returns unsigned");
static_assert(std::is_same<decltype(compile_time.file_name()), const char*>::value, "file_name() returns const char*");
static_assert(std::is_same<decltype(compile_time.function_name()), const char*>::value, "function_name() returns const char*");
static_assert(noexcept(compile_time.function_name()), "function_name() is noexcept");

//Compile-time part: the value stays cheap to pass around.
static_assert(std::is_trivially_copyable<functional>::value, "the object stays trivially copyable");
static_assert(std::is_nothrow_copy_constructible<functional>::value, "the copy constructor is noexcept");
static_assert(std::is_nothrow_move_assignable<functional>::value, "the move assignment is noexcept");

//Compile-time part: a location can only be produced at a call site.
static_assert(!std::is_default_constructible<functional>::value, "the default constructor stays deleted");

void takes_const_ref(const functional& loc, unsigned expected_line)
{
	CHECK(loc.line() == expected_line);
}

struct probe
{
	void run()
	{
		CEPO_FUNCTION_SOURCE_LOCATION(here);
		constexpr unsigned here_line = __LINE__ - 1;
		CHECK(here.line() == here_line);
		CHECK(std::strcmp(here.file_name(), __FILE__) == 0);
		CHECK(std::strstr(here.function_name(), "run") != nullptr);
		CHECK(std::strstr(here.function_name(), "probe") != nullptr);
	}
};

template <typename T>
void templated_probe(T)
{
	CEPO_FUNCTION_SOURCE_LOCATION(here);
	CHECK(std::strstr(here.function_name(), "templated_probe") != nullptr);
}

auto main() -> int
{
	//Run-time part: inside main the recorded function is main itself.
	CEPO_FUNCTION_SOURCE_LOCATION(fs);
	constexpr unsigned fs_line = __LINE__ - 1;
	CHECK(fs.line() == fs_line);
	CHECK(std::strcmp(fs.file_name(), __FILE__) == 0);
	CHECK(std::strstr(fs.file_name(), "test.cpp") != nullptr);
	CHECK(std::strstr(fs.function_name(), "main") != nullptr);

	//Run-time part: the value form records the same three members.
	functional by_current = CEPO_CURRENT_FUNCTION_SOURCE_LOCATION();
	constexpr unsigned by_current_line = __LINE__ - 1;
	CHECK(by_current.line() == by_current_line);
	CHECK(std::strcmp(by_current.function_name(), fs.function_name()) == 0);

	//Run-time part: the value form binds to a const reference.
	takes_const_ref(CEPO_CURRENT_FUNCTION_SOURCE_LOCATION(), __LINE__);

	//Run-time part: both classes read the same __LINE__ at adjacent call sites.
	cepo::anywhere_source_location anywhere_here = CEPO_CURRENT_ANYWHERE_SOURCE_LOCATION();
	functional function_here = CEPO_CURRENT_FUNCTION_SOURCE_LOCATION();
	CHECK(function_here.line() == anywhere_here.line() + 1);

	//Run-time part: copying keeps all three members.
	functional copied = fs;
	CHECK(copied.line() == fs.line());
	CHECK(std::strcmp(copied.file_name(), fs.file_name()) == 0);
	CHECK(std::strcmp(copied.function_name(), fs.function_name()) == 0);

	//Run-time part: assignment replaces all three members.
	copied = functional{__FILE__, "assigned_name", 42u};
	CHECK(copied.line() == 42u);
	CHECK(std::strcmp(copied.file_name(), __FILE__) == 0);
	CHECK(std::strcmp(copied.function_name(), "assigned_name") == 0);

	//Run-time part: the member function shape carries both names.
	probe p;
	p.run();

	//Run-time part: the template shape still carries the function name.
	templated_probe(1);

	//Run-time part: a lambda has no name of its own, so only non-emptiness is
	//portable here.
	auto nameless = []()
	{
		CEPO_FUNCTION_SOURCE_LOCATION(here);
		return here.function_name() != nullptr and here.function_name()[0] != '\0';
	};
	CHECK(nameless());

	std::printf("function: %s\n", g_fail == 0 ? "ok" : "failed");
	return g_fail == 0 ? 0 : 1;
}
