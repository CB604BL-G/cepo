//Must not compile: operator!= is constrained on
//is_callable<comparator::not_equal_to, const T&, const T&>, which is checked
//independently of the equality constraint. A type that only provides
//operator== therefore supports == but not !=.
#include "cepo/result.hpp"
#include "cepo/result/factory.hpp"

struct equality_only
{
	int value;

	bool operator==(const equality_only& other) const noexcept
	{
		return value == other.value;
	}
};

auto main() -> int
{
	cepo::result<equality_only, int> a = cepo::make_result_value<equality_only>();
	cepo::result<equality_only, int> b = cepo::make_result_value<equality_only>();

	bool equal = (a == b);		//fine
	return (a != b) || equal;	//must not compile
}
