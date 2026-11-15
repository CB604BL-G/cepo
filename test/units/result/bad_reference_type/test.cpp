//Must not compile: T has to be an object type, a reference is rejected by the
//static_assert in cepo::result.
#include "cepo/result.hpp"
#include "cepo/result/factory.hpp"

auto main() -> int
{
	cepo::result<int&, int> r = cepo::make_result_value<int>(1);
	(void)r;
	return 0;
}
