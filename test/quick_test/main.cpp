//#include "cepo/type_traits/is_bool_testable.hpp"
#include "cepo/type_traits/is_nothrow_bool_testable.hpp"

class t
{
public:
	operator bool() noexcept
	{
		return true;
	}

	bool operator not() noexcept;
};

auto main() -> int
{
	t o;
	bool i = bool(o);
	const t co;
	static_assert(cepo::is_nothrow_bool_testable<t>::value, "");
// 非 const operator bool 在 const 左值上不可用
//static_assert(cepo::is_bool_testable<t>::value, "");
}
