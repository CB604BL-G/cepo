//#include "cepo/type_traits/is_bool_testable.hpp"
#include "cepo/type_traits/is_equality_comparison_operable_with.hpp"
#include "cepo/type_traits/is_nothrow_bool_testable.hpp"
#include "cepo/type_traits/is_oneway_equality_comparison_operable_with.hpp"

class t
{
public:
	operator bool() noexcept
	{
		return true;
	}

	bool operator not() noexcept;

	bool operator==(const t&) const;
};

bool operator==(t, int);
bool operator==(int, t);
bool operator!=(t, int);
bool operator!=(int, t);


auto main() -> int
{
	static_assert(
		cepo::is_equality_comparison_operable_with<t, int>::value,
		"");
}
