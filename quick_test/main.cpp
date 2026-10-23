#include "cepo/macros/nodiscard.hpp"
#include "cepo/result/slot_base.hpp"
#include "cepo/utility/as_const.hpp"
#include <cepo/result.hpp>
#include <cstdio>
#include <string>
#include <utility>

void foo(const int&) {}

CEPO_NODISCARD
int fooi() {return 0;}

auto main() -> int
{
	cepo::result<void, std::string> r = cepo::result_error_type<std::string>("error");
	cepo::as_const(r)
		.ok([](){std::printf("s\n");})
		.error([](const std::string& r){std::printf("%s\n", r.c_str());});
	r.is_ok();

	decltype(r) rr = std::move(r);
	rr.ok([](){std::printf("s\n");})
		.error([](const std::string& r){std::printf("%s\n", r.c_str());});

	r.ok([](){std::printf("s\n");})
		.error([](const std::string& r){std::printf("%s\n", r.c_str());});

	r.unwrap_error();

}
