//#include "cepo/type_traits/is_bool_testable.hpp"
#include "cepo/type_traits/is_nothrow_oneway_equality_operable_with.hpp"
#include "cepo/type_traits/is_nothrow_oneway_inequality_operable_with.hpp"
#include "cepo/type_traits/is_oneway_equality_comparison_operable_with.hpp"
#include "cepo/type_traits/is_oneway_equality_operable_with.hpp"
#include "cepo/type_traits/is_oneway_inequality_operable_with.hpp"
class t
{
public:
};

bool operator==(t, int) noexcept;
bool operator==(int, t);
bool operator!=(t, int) noexcept;
bool operator!=(int, t);


auto main() -> int
{
	static_assert(
		cepo::is_oneway_equality_comparison_operable_with<t, int>::value,
		"");
}
