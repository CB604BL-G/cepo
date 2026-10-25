#include "cepo/result.hpp"
#include "cepo/result/factory.hpp"
#include <cstdio>

void foo(const int&) {}

void fooi() { std::puts("ok branch"); }

struct t
{
	int r;
	t(int rr) : r(rr) {} 
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
}
