//Must not compile: E has to be an object type, void is rejected by the
//static_assert in cepo::result and in cepo::result_error_slot.
#include "cepo/result.hpp"
#include "cepo/result/factory.hpp"

auto main() -> int
{
	cepo::result<int, void> r = cepo::make_result_error<void>();
	(void)r;
	return 0;
}
