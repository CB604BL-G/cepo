#include "cepo/result.hpp"
#include "cepo/result/factory.hpp"
class t
{
public:
	bool operator==(const t&) = delete;// const noexcept { return true; };
};

auto main() -> int
{
	cepo::result<int, t> r = cepo::make_result_error<t>();
	r.is_equal_to(r);
}
