//Must not compile: the operands of operator== have to be the same result type,
//because the friend template requires both parameters to deduce to T and E.
#include "cepo/result.hpp"
#include "cepo/result/factory.hpp"

auto main() -> int
{
	cepo::result<int, int> a = cepo::make_result_value<int>(1);
	cepo::result<long, int> b = cepo::make_result_value<long>(1L);
	return (a == b) ? 0 : 1;
}
