#include "cepo/result.hpp"
#include "cepo/result/factory.hpp"
#include <cstdio>
class t
{
public:
	bool operator==(const t&)const noexcept { return true; };
};

auto main() -> int
{
	cepo::result<const void, t> r = cepo::make_result_error<t>();
	cepo::result<const void, t> rr = cepo::make_result_error<t>();

	std::printf("%d\n", r == rr);
}
