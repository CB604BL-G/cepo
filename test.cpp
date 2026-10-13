#include "memory/make_unique.hpp"
#include "type_traits/conjunction.hpp"
#include "type_traits/disjunction.hpp"
#include "type_traits/is_bounded_array.hpp"
#include "type_traits/is_nothrow_convertible.hpp"
#include "type_traits/negation.hpp"
#include <string>
#include <type_traits>

struct t
{
	t(int)	 noexcept
	{
		
	}
};

auto main() -> int
{
	auto str = cepo::make_unique<std::string[5]>();
	static_assert(cepo::is_nothrow_convertible<int, t>::value, "");
}
