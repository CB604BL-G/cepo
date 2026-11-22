//Contract of cepo::anywhere_source_location: the file name and the line are
//read at the call site, the recorded pair is usable in a constant expression,
//and the value stays a cheap trivially-copyable object that has no default
//constructor.
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

using anywhere = cepo::anywhere_source_location;

//Compile-time part: both accessors are constexpr, so the recorded pair can be
//consumed by a static_assert.
constexpr anywhere compile_time{__FILE__, 7u};
static_assert(compile_time.line() == 7u, "the line is stored verbatim");
static_assert(compile_time.file_name() != nullptr, "the file name is not null");
static_assert(compile_time.file_name()[0] != '\0', "the file name is not empty");
static_assert(std::is_same<decltype(compile_time.line()), unsigned>::value, "line() returns unsigned");
static_assert(std::is_same<decltype(compile_time.file_name()), const char*>::value, "file_name() returns const char*");
static_assert(noexcept(compile_time.line()), "line() is noexcept");
static_assert(noexcept(compile_time.file_name()), "file_name() is noexcept");

//Compile-time part: passing one around stays cheap and cannot throw.
static_assert(std::is_trivially_copyable<anywhere>::value, "the object stays trivially copyable");
static_assert(std::is_nothrow_copy_constructible<anywhere>::value, "the copy constructor is noexcept");
static_assert(std::is_nothrow_move_constructible<anywhere>::value, "the move constructor is noexcept");
static_assert(std::is_nothrow_copy_assignable<anywhere>::value, "the copy assignment is noexcept");
static_assert(std::is_nothrow_move_assignable<anywhere>::value, "the move assignment is noexcept");

//Compile-time part: a location can only be produced at a call site.
static_assert(!std::is_default_constructible<anywhere>::value, "the default constructor stays deleted");

void takes_const_ref(const anywhere& loc, unsigned expected_line)
{
	CHECK(loc.line() == expected_line);
}

auto main() -> int
{
	//Run-time part: the value form records the line it is written on.
	anywhere here = CEPO_CURRENT_ANYWHERE_SOURCE_LOCATION();
	constexpr unsigned here_line = __LINE__ - 1;
	CHECK(here.line() == here_line);
	CHECK(std::strcmp(here.file_name(), __FILE__) == 0);
	CHECK(std::strstr(here.file_name(), "test.cpp") != nullptr);

	//Run-time part: the named form puts the variable in the current scope.
	CEPO_ANYWHERE_SOURCE_LOCATION(named);
	constexpr unsigned named_line = __LINE__ - 1;
	CHECK(named.line() == named_line);
	CHECK(std::strcmp(named.file_name(), here.file_name()) == 0);

	//Run-time part: two locations written on adjacent lines differ by one, so
	//the line really comes from the call site and not from the header.
	CEPO_ANYWHERE_SOURCE_LOCATION(first);
	CEPO_ANYWHERE_SOURCE_LOCATION(second);
	CHECK(second.line() == first.line() + 1);

	//Run-time part: the value form is an rvalue and binds to a const reference.
	takes_const_ref(CEPO_CURRENT_ANYWHERE_SOURCE_LOCATION(), __LINE__);

	//Run-time part: copying keeps both members.
	anywhere copied = here;
	CHECK(copied.line() == here.line());
	CHECK(std::strcmp(copied.file_name(), here.file_name()) == 0);

	//Run-time part: assignment replaces both members.
	copied = anywhere{__FILE__, 42u};
	CHECK(copied.line() == 42u);
	CHECK(std::strcmp(copied.file_name(), __FILE__) == 0);

	anywhere assigned = here;
	assigned = anywhere{__FILE__, 9u};
	CHECK(assigned.line() == 9u);
	CHECK(std::strcmp(assigned.file_name(), __FILE__) == 0);

	std::printf("anywhere: %s\n", g_fail == 0 ? "ok" : "failed");
	return g_fail == 0 ? 0 : 1;
}
