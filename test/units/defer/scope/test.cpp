//Contract of CEPO_DEFER: the block becomes a callback, it captures the
//surrounding scope by reference, and it runs when that scope ends. Several of
//them in one scope run in reverse order, and one written inside another runs
//while the outer callback runs.
//
//The checks below observe the order of execution through a trace of marks
//appended by the callbacks, so a wrong order is reported with both the trace
//that happened and the trace that was expected.
#include "cepo/defer.hpp"

#include <cstdio>
#include <cstring>

namespace
{
	int g_fail = 0;

	char trace[32];
	unsigned trace_length = 0;

	void record(char mark)
	{
		trace[trace_length] = mark;
		++trace_length;
	}

	void clear_trace()
	{
		trace_length = 0;
	}

	const char* trace_text()
	{
		trace[trace_length] = '\0';
		return trace;
	}

	void check(bool cond, const char* expr, int line)
	{
		if (!cond)
		{
			std::printf("FAIL line %d: %s\n", line, expr);
			++g_fail;
		}
	}

	void check_trace(const char* expected, int line)
	{
		if (std::strcmp(trace_text(), expected) == 0)
		{
			return;
		}

		std::printf(
			"FAIL line %d: trace is \"%s\", expected \"%s\"\n",
			line, trace_text(), expected
		);
		++g_fail;
	}
}

#define CHECK(expr) check((expr), #expr, __LINE__)
#define CHECK_TRACE(expected) check_trace((expected), __LINE__)

//The callback has to run on the early return path as well, and before the
//caller receives control back.
auto leave_early(bool stop) -> int
{
	CEPO_DEFER
	{
		record('r');
	};

	if (stop)
	{
		return 1;
	}

	record('f');
	return 0;
}

auto main() -> int
{
	//Callbacks in one scope run in reverse order, and they see the captured
	//variable as it stands when the scope ends, not as it stood next to them.
	int captured = 0;
	clear_trace();
	{
		CEPO_DEFER
		{
			record('a');
			captured += 1;
		};

		CEPO_DEFER
		{
			record('b');
			captured += 10;
		};

		captured = 5;
		CHECK_TRACE("");
	}

	CHECK_TRACE("ba");
	CHECK(captured == 16);

	//A callback writes through the reference it captured.
	int touched = 0;
	{
		CEPO_DEFER
		{
			touched = 42;
		};

		CHECK(touched == 0);
	}

	CHECK(touched == 42);

	//A callback belongs to the scope it was written in, so it runs at the end
	//of every iteration of the loop body.
	clear_trace();
	for (int i = 0; i < 3; ++i)
	{
		CEPO_DEFER
		{
			record('i');
		};

		record('b');
	}

	CHECK_TRACE("bibibi");

	//Leaving a function early runs the callback too, after the return value is
	//already fixed and before the caller continues.
	clear_trace();
	CHECK(leave_early(true) == 1);
	CHECK_TRACE("r");

	clear_trace();
	CHECK(leave_early(false) == 0);
	CHECK_TRACE("fr");

	//A bare return inside the block leaves the block only, not the function
	//around it, which is why the rest of that scope keeps running.
	clear_trace();
	{
		CEPO_DEFER
		{
			record('x');
			return;
			record('y');
		};
	}

	CHECK_TRACE("x");

	//An inner block ends before the rest of the outer scope runs, so its
	//callback fires first, the outer scope then continues, and the callback of
	//the outer scope fires last.
	clear_trace();
	{
		CEPO_DEFER
		{
			record('o');
		};

		{
			CEPO_DEFER
			{
				record('i');
			};
		}

		record('m');
	}

	CHECK_TRACE("imo");

	//A defer written inside another one runs while the outer callback runs:
	//after the rest of that callback, before the outer callback returns.
	clear_trace();
	{
		CEPO_DEFER
		{
			record('1');

			CEPO_DEFER
			{
				record('3');
			};

			record('2');
		};

		record('0');
	}

	CHECK_TRACE("0123");

	//The inner callback sees what the outer callback did to a shared variable.
	int nested = 0;
	{
		CEPO_DEFER
		{
			nested = 1;

			CEPO_DEFER
			{
				nested = nested * 10 + 3;
			};

			nested = nested * 10 + 2;
		};
	}

	CHECK(nested == 123);

	std::printf("scope: %s\n", g_fail == 0 ? "ok" : "failed");
	return g_fail == 0 ? 0 : 1;
}
