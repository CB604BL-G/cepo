#include "cepo/source_location.hpp"
#include "cepo/defer.hpp"
#include <cstdio>

auto main() -> int
{
	cepo::anywhere_source_location s = CEPO_CURRENT_ANYWHERE_SOURCE_LOCATION();
	std::printf("%s, %d\n", s.file_name(), s.line());

	CEPO_ANYWHERE_SOURCE_LOCATION(ss);
	std::printf("%s, %d\n", ss.file_name(), ss.line());

	CEPO_FUNCTION_SOURCE_LOCATION(fs);
	printf("%s\n", fs.function_name());

	CEPO_DEFER
	{
		printf("Defer do\n");
	};

	CEPO_DEFER
	{
		CEPO_DEFER
		{
			printf("%s\n", fs.function_name());
		};

		printf("Completed\n");
	};
}
