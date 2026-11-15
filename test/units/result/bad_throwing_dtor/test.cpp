//Must not compile: both stored types need a nothrow destructor, because the
//result destroys them from its own noexcept destructor and move assignment.
#include "cepo/result.hpp"
#include "cepo/result/factory.hpp"

struct throwing_dtor
{
	~throwing_dtor() noexcept(false) {}
};

auto main() -> int
{
	cepo::result<throwing_dtor, int> r = cepo::make_result_value<throwing_dtor>();
	(void)r;
	return 0;
}
