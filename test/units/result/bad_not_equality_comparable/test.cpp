//Must not compile: operator== is constrained on is_equality_operable, so a
//stored type without a usable operator== makes the comparison unavailable
//instead of failing somewhere inside the comparison itself.
#include "cepo/result.hpp"
#include "cepo/result/factory.hpp"

struct no_equality
{
	int value;
};

auto main() -> int
{
	cepo::result<no_equality, int> a = cepo::make_result_value<no_equality>();
	cepo::result<no_equality, int> b = cepo::make_result_value<no_equality>();
	return (a == b) ? 0 : 1;
}
