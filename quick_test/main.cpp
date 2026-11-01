#include "cepo/callable/traits.hpp"

auto main() -> int
{
	using t = cepo::callable_traits<decltype(main)>;
	using tt = t::class_type;
}
