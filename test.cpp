#include "cepo/result.hpp"
#include <cstdio>
#include <string>
#include <utility>
#include "cepo/result/slot_base.hpp"

auto main() -> int
{
	cepo::result_value_type<std::string> i("123");
	decltype(i) j = std::move(i);
	
}
