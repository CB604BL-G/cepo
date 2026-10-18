#include "cepo/result.hpp"
#include <cstdio>
#include <stdexcept>
#include <string>
#include <utility>
#include "cepo/result/slot_base.hpp"

struct t
{
	int i;

	t(int ii): i(ii)
	{
		throw std::logic_error{"Boom"};
	}
};

auto main() -> int
{
	cepo::result<t, int> i{cepo::result_value_type<t>(123)};
	
	printf("%d\n", i.has_exception());
}
