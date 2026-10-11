#include "memory/make_unique.hpp"
#include "type_traits/is_bounded_array.hpp"
#include <string>
#include <type_traits>

struct t
{
	//t() = delete;
};

auto main() -> int
{
	auto str = cepo::make_unique<t[5]>();
	//static_assert(std::is_default_constructible<t>::value, "");
}
