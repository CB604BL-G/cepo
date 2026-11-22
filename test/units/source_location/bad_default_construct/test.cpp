//Must not compile: the default constructor of both location classes is
//deleted, so a location can only be produced by a macro at a call site.
#include "cepo/source_location.hpp"

auto main() -> int
{
	cepo::anywhere_source_location nowhere;
	(void)nowhere;
	return 0;
}
