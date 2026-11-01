#include "cepo/result.hpp"
#include "cepo/result/factory.hpp"
#include "cepo/type_traits/is_move_only_constructible.hpp"
#include <cstdio>
#include <type_traits>

void foo(const int&) {}

void fooi() { std::puts("ok branch"); }

struct t
{
	int r;
	t(int rr) : r(rr) {}
	t(t&&) {}
	//t(t&) {}
	t(const t&) = delete;
	//t() noexcept = delete;
};

auto main() -> int
{
	cepo::result<void, t> r = cepo::make_result_error<t>(12);
	
	r.ok(fooi).ignore_error();
	/*	.error(
		[](const t& tt)
		{
			std::printf("%d\n", tt.r);
		}
	);*/
	t i = 10;
}

static_assert(cepo::is_move_only_constructible<t>::value, "");

static_assert(std::is_constructible<t, t&&>::value and not std::is_constructible<t, const t&>::value, "");
