#include "cepo/result.hpp"
#include <cstdio>

auto main() -> int
{
	cepo::result<int, float> e =
		cepo::make_result_error<float>(12);

	e
	.ok(
		[](int& v)
		{
			printf("%d\n", v);
		}
	)
	.error(
		[](float& f)
		{
			printf("%f\n", f);
		}
	);
}
