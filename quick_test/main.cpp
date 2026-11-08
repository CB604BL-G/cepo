// quick_test/main.cpp -- scratch harness for cepo::result
// owned by the doc/script side; do not treat as a permanent test

#include "cepo/result.hpp"
#include "cepo/result/factory.hpp"
#include "cepo/result/bad_result_unwrap.hpp"
#include "cepo/result/slots.hpp"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <exception>
#include <string>
#include <stdexcept>
#include <utility>

// cepo::result<void, E> currently cannot be built from
// cepo::make_result_error<E>(...): commit 8fc49ef dropped
// "friend class impl::result_base_void;" from cepo::result_error_slot,
// so every void-result error path fails to compile.
// Set to 1 to test those paths again once the header is fixed.
#ifndef CEPO_TEST_VOID_ERROR_SLOT
#define CEPO_TEST_VOID_ERROR_SLOT 1
#endif

static int g_pass = 0;
static int g_fail = 0;

static void check(bool cond, const char* what)
{
	if (cond) { ++g_pass; }
	else { ++g_fail; std::printf("  [FAIL] %s\n", what); }
}

struct raiser
{
	static int live;
	raiser() { ++live; }
	//must throw *before* touching live: the object never finishes
	//construction, so its destructor will never run
	raiser(raiser&&) { throw std::runtime_error("move boom"); }
	raiser& operator=(raiser&&) { throw std::runtime_error("move-assign boom"); }
	raiser(const raiser&) = delete;
	raiser& operator=(const raiser&) = delete;
	~raiser() noexcept { --live; }
};
int raiser::live = 0;

struct probe
{
	static int live;
	static int moved;
	int v;
	explicit probe(int x): v(x) { ++live; }
	probe(probe&& o) noexcept: v(o.v) { o.v = -1; ++live; ++moved; }
	probe& operator=(probe&& o) noexcept { v = o.v; o.v = -1; ++moved; return *this; }
	probe(const probe&) = delete;
	probe& operator=(const probe&) = delete;
	~probe() noexcept { --live; }
};
int probe::live = 0;
int probe::moved = 0;

struct alignas(64) aligned64
{
	unsigned char pad[64];
	int v;
	explicit aligned64(int x): pad(), v(x) {}
};

static void take_int_ref(int& v) { v += 1; }

struct lv_functor
{
	int* out;
	void operator()(int& v) const { *out = v; }
};

struct throw_on_copy_ctor
{
	throw_on_copy_ctor() = default;
	throw_on_copy_ctor(const throw_on_copy_ctor&) { throw std::runtime_error("ctor boom"); }
};

int main()
{
	// ---------- 1. value state ----------
	std::printf("[1] value state\n");
	{
		cepo::result<int, int> r = cepo::make_result_value<int>(42);
		check(r.is_ok(), "is_ok");
		check(!r.is_error(), "!is_error");
		check(!r.is_empty(), "!is_empty");
		check(!r.has_exception(), "!has_exception");
		check(r.unwrap_value() == 42, "unwrap_value == 42");
	}

	// ---------- 2. error state ----------
	std::printf("[2] error state\n");
	{
		cepo::result<int, int> r = cepo::make_result_error<int>(7);
		check(r.is_error(), "is_error");
		check(!r.is_ok(), "!is_ok");
		check(r.unwrap_error() == 7, "unwrap_error == 7");
	}

	// ---------- 3. unwrap on wrong state ----------
	std::printf("[3] unwrap on wrong state\n");
	{
		cepo::result<int, int> r = cepo::make_result_error<int>(7);
		bool threw = false;
		cepo::bad_result_unwrap::reason_type reason =
			cepo::bad_result_unwrap::reason_type::is_empty;
		try { (void)r.unwrap_value(); }
		catch (const cepo::bad_result_unwrap& e) { threw = true; reason = e.reason(); }
		check(threw, "unwrap_value on error throws");
		check(reason == cepo::bad_result_unwrap::reason_type::is_error, "reason == is_error");
	}
	{
		cepo::result<int, int> r = cepo::make_result_value<int>(1);
		bool threw = false;
		cepo::bad_result_unwrap::reason_type reason =
			cepo::bad_result_unwrap::reason_type::is_empty;
		try { (void)r.unwrap_error(); }
		catch (const cepo::bad_result_unwrap& e) { threw = true; reason = e.reason(); }
		check(threw, "unwrap_error on ok throws");
		check(reason == cepo::bad_result_unwrap::reason_type::is_ok, "reason == is_ok");
	}

	// ---------- 4. move leaves source empty ----------
	std::printf("[4] move leaves source empty\n");
	{
		cepo::result<std::string, int> a = cepo::make_result_value<std::string>("hello");
		cepo::result<std::string, int> b(std::move(a));
		check(a.is_empty(), "source is_empty after move-ctor");
		check(b.is_ok() && b.unwrap_value() == "hello", "target holds value");

		bool threw = false;
		cepo::bad_result_unwrap::reason_type reason =
			cepo::bad_result_unwrap::reason_type::is_ok;
		try { (void)a.unwrap_value(); }
		catch (const cepo::bad_result_unwrap& e) { threw = true; reason = e.reason(); }
		check(threw, "unwrap on moved-from throws");
		check(reason == cepo::bad_result_unwrap::reason_type::is_empty, "reason == is_empty");
	}

	// ---------- 5. move assignment ----------
	std::printf("[5] move assignment\n");
	{
		cepo::result<int, int> a = cepo::make_result_value<int>(1);
		cepo::result<int, int> b = cepo::make_result_error<int>(2);

		a = std::move(b);
		check(a.is_error() && a.unwrap_error() == 2, "assign error over value");
		check(b.is_empty(), "rhs is_empty after move-assign");

		cepo::result<int, int> c = cepo::make_result_value<int>(3);
		a = std::move(c);
		check(a.is_ok() && a.unwrap_value() == 3, "assign value over error");
	}

	// ---------- 6. self move assignment ----------
	std::printf("[6] self move assignment\n");
	{
		cepo::result<int, int> a = cepo::make_result_value<int>(5);
		cepo::result<int, int>& same = a;
		a = std::move(same);
		check(a.is_ok() && a.unwrap_value() == 5, "self-move keeps value");
	}

	// ---------- 7. ok() chain, value branch ----------
	std::printf("[7] ok() value branch\n");
	{
		cepo::result<int, int> r = cepo::make_result_value<int>(10);
		int got = 0;
		int err = -1;
		r.ok([&](int& v) { got = v; })
		 .error([&](const int& e) { err = e; });
		check(got == 10, "ok callback ran with value");
		check(err == -1, "error callback did not run");
	}
	// ---------- 8. ok() chain, error branch ----------
	std::printf("[8] ok() error branch\n");
	{
		cepo::result<int, int> r = cepo::make_result_error<int>(99);
		bool ok_ran = false;
		int err = -1;
		r.ok([&](int&) { ok_ran = true; })
		 .error([&](const int& e) { err = e; });
		check(!ok_ran, "ok callback skipped");
		check(err == 99, "error callback ran with error");
	}

	// ---------- 9. ignore_error ----------
	std::printf("[9] ignore_error\n");
	{
		cepo::result<int, int> r = cepo::make_result_error<int>(1);
		r.ok([](int&) {}).ignore_error();
		check(true, "ignore_error callable on error");
	}

	// ---------- 10. ok() on moved-from (empty) ----------
	std::printf("[10] ok() on empty\n");
	{
		cepo::result<int, int> a = cepo::make_result_value<int>(1);
		cepo::result<int, int> b(std::move(a));
		bool ok_ran = false;
		bool err_ran = false;
		a.ok([&](int&) { ok_ran = true; })
		 .error([&](const int&) { err_ran = true; });
		check(!ok_ran, "ok callback skipped on empty");
		check(!err_ran, "error callback skipped on empty (empty is silent)");
	}

	// ---------- 11. void result ----------
	std::printf("[11] void result\n");
	{
		cepo::result<void, int> r = cepo::make_result_value<void>();
		check(r.is_ok(), "void is_ok");
		bool ran = false;
		r.ok([&]() { ran = true; }).ignore_error();
		check(ran, "void ok callback ran");

#if CEPO_TEST_VOID_ERROR_SLOT
		cepo::result<void, int> e = cepo::make_result_error<int>(3);
		check(e.is_error() && e.unwrap_error() == 3, "void error state");
		bool ok_ran = false;
		int err = -1;
		e.ok([&]() { ok_ran = true; }).error([&](const int& v) { err = v; });
		check(!ok_ran && err == 3, "void error branch");
#else
		std::printf("  (void error-slot cases gated by CEPO_TEST_VOID_ERROR_SLOT)\n");
#endif
	}

	// ---------- 12. throwing T constructor during slot build ----------
	std::printf("[12] throwing constructor in slot\n");
	{
		cepo::result<throw_on_copy_ctor, int> r =
			cepo::make_result_value<throw_on_copy_ctor>(throw_on_copy_ctor());
		check(r.is_empty(), "state empty after ctor throw");
		check(r.has_exception(), "has_exception true");
		check(r.get_exception() != nullptr, "get_exception non-null");
		bool threw = false;
		try { (void)r.unwrap_value(); }
		catch (const cepo::bad_result_unwrap&) { threw = true; }
		check(threw, "unwrap on empty throws");
	}

	// ---------- 13. throwing move during result transfer ----------
	std::printf("[13] throwing move during transfer\n");
	{
		// the slot->result transfer is a move, so a T with a throwing
		// move constructor can never produce an ok result at all
		check(raiser::live == 0, "raiser baseline zero");
		cepo::result<raiser, int> r = cepo::make_result_value<raiser>();
		check(r.is_empty(), "empty after throwing move during construction");
		check(!r.is_ok(), "never becomes ok");
		check(r.has_exception(), "exception recorded");
		check(raiser::live == 0, "no raiser leaked");
	}

	// ---------- 14. cross-state move assign with throwing move ----------
	std::printf("[14] throwing move in move-assign\n");
	{
		cepo::result<raiser, int> a = cepo::make_result_error<int>(1);
		cepo::result<raiser, int> b = cepo::make_result_value<raiser>();
		check(b.is_empty() && b.has_exception(), "b already empty+exception");
		a = std::move(b);
		check(a.is_empty(), "empty after move-assign from empty");
		check(a.has_exception(), "exception propagated by move-assign");
	}

	// ---------- 15. cv-void specialisations accept make_result_value<void>() ----------
#if defined(CEPO_SKIP_CV_VOID)
	std::printf("[15] cv-void (skipped)\n");
#else
	std::printf("[15] cv-void\n");
	{
		cepo::result<const void, int> cv = cepo::make_result_value<const void>();
		check(cv.is_ok(), "const void ok");
	}
#endif

	// ---------- 16. const result uses the const ok() overload ----------
	std::printf("[16] const ok()\n");
	{
		cepo::result<int, int> mut = cepo::make_result_value<int>(21);
		const cepo::result<int, int>& c = mut;
		int got = 0;
		c.ok([&](const int& v) { got = v; }).ignore_error();
		check(got == 21, "const ok() passes const T&");

		const cepo::result<int, int> ce = cepo::make_result_error<int>(4);
		int err = -1;
		ce.ok([](const int&) {}).error([&](const int& e) { err = e; });
		check(err == 4, "const ok() still forwards error");
	}

	// ---------- 17. callback shapes: lvalue functor / function pointer ----------
	std::printf("[17] callback shapes\n");
	{
		cepo::result<int, int> r = cepo::make_result_value<int>(8);
		int got = -1;
		lv_functor f{&got};
		r.ok(f).ignore_error();
		check(got == 8, "lvalue functor accepted");

		cepo::result<int, int> r2 = cepo::make_result_value<int>(8);
		void (*fp)(int&) = &take_int_ref;
		r2.ok(fp).ignore_error();
		check(r2.unwrap_value() == 9, "function pointer accepted and mutates value");
	}

	// ---------- 18. void result move matrix ----------
	std::printf("[18] void move matrix\n");
	{
		cepo::result<void, int> a = cepo::make_result_value<void>();
		cepo::result<void, int> b(std::move(a));
		check(b.is_ok(), "void move-ctor keeps ok");
		check(a.is_empty(), "void source empty after move-ctor");
		check(!b.has_exception(), "void move-ctor has no exception");

#if CEPO_TEST_VOID_ERROR_SLOT
		cepo::result<void, int> e = cepo::make_result_error<int>(5);
		cepo::result<void, int> f = cepo::make_result_value<void>();
		e = std::move(f);
		check(e.is_ok() && !e.has_exception(), "void assign ok over error");
		check(f.is_empty(), "void rhs empty after assign");

		cepo::result<void, int> g = cepo::make_result_value<void>();
		cepo::result<void, int> h = cepo::make_result_error<int>(6);
		g = std::move(h);
		check(g.is_error() && g.unwrap_error() == 6, "void assign error over ok");
		check(h.is_empty(), "void rhs empty after error assign");
#endif

		cepo::result<void, int> i = cepo::make_result_value<void>();
		cepo::result<void, int> j = cepo::make_result_value<void>();
		i = std::move(j);
		check(i.is_ok() && j.is_empty(), "void assign ok over ok");
	}

	// ---------- 19. void self move assignment ----------
	std::printf("[19] void self move assign\n");
	{
		cepo::result<void, int> v = cepo::make_result_value<void>();
		cepo::result<void, int>& vr = v;
		v = std::move(vr);
		check(v.is_ok(), "void self-move keeps ok");

#if CEPO_TEST_VOID_ERROR_SLOT
		cepo::result<void, int> w = cepo::make_result_error<int>(9);
		cepo::result<void, int>& wr = w;
		w = std::move(wr);
		check(w.is_error() && w.unwrap_error() == 9, "void self-move keeps error");
#endif
	}

	// ---------- 20. const void result ----------
	std::printf("[20] const void\n");
	{
		cepo::result<const void, int> c = cepo::make_result_value<const void>();
		check(c.is_ok() && !c.has_exception(), "const void ok");
		bool ran = false;
		c.ok([&]() { ran = true; }).ignore_error();
		check(ran, "const void ok callback");

#if CEPO_TEST_VOID_ERROR_SLOT
		cepo::result<const void, int> ce = cepo::make_result_error<int>(11);
		check(ce.is_error() && ce.unwrap_error() == 11, "const void error state");
		bool ok_ran = false;
		ce.ok([&]() { ok_ran = true; }).ignore_error();
		check(!ok_ran, "const void error skips ok callback");
#endif
	}

	// ---------- 21. volatile void result ----------
	std::printf("[21] volatile void\n");
	{
		cepo::result<volatile void, int> v = cepo::make_result_value<volatile void>();
		check(v.is_ok(), "volatile void ok");

#if CEPO_TEST_VOID_ERROR_SLOT
		cepo::result<volatile void, int> ve = cepo::make_result_error<int>(12);
		bool threw = false;
		try { (void)ve.unwrap_error(); }
		catch (...) { threw = true; }
		check(!threw && ve.unwrap_error() == 12, "volatile void unwrap_error");
#endif
	}

	// ---------- 22. over-aligned T ----------
	std::printf("[22] alignment\n");
	{
		cepo::result<aligned64, int> r = cepo::make_result_value<aligned64>(9);
		check(reinterpret_cast<std::uintptr_t>(&r.unwrap_value()) % 64 == 0,
			"value is 64-byte aligned");
		check(r.unwrap_value().v == 9, "aligned value preserved");

		const cepo::result<aligned64, int>& cr = r;
		check(reinterpret_cast<std::uintptr_t>(&cr.unwrap_value()) % 64 == 0,
			"const value is 64-byte aligned");
	}

	// ---------- 23. mutate through unwrap_value() ----------
	std::printf("[23] mutate through unwrap_value\n");
	{
		cepo::result<int, int> r = cepo::make_result_value<int>(1);
		r.unwrap_value() += 41;
		check(r.unwrap_value() == 42, "non-const unwrap_value is writable");
		check(r.is_ok(), "state unchanged after write");
	}

	// ---------- 24. what() text and reason mapping ----------
	std::printf("[24] bad_result_unwrap text\n");
	{
		cepo::bad_result_unwrap a(cepo::bad_result_unwrap::reason_type::is_ok);
		cepo::bad_result_unwrap b(cepo::bad_result_unwrap::reason_type::is_error);
		cepo::bad_result_unwrap c(cepo::bad_result_unwrap::reason_type::is_empty);
		check(a.reason() == cepo::bad_result_unwrap::reason_type::is_ok, "reason is_ok");
		check(b.reason() == cepo::bad_result_unwrap::reason_type::is_error, "reason is_error");
		check(c.reason() == cepo::bad_result_unwrap::reason_type::is_empty, "reason is_empty");
		check(std::strlen(a.what()) > 0, "what() non-empty");
		check(std::strcmp(a.what(), b.what()) != 0 &&
			std::strcmp(b.what(), c.what()) != 0 &&
			std::strcmp(a.what(), c.what()) != 0, "what() differs per reason");

		//copy/assign of the exception object
		cepo::bad_result_unwrap d(b);
		check(d.reason() == b.reason(), "copy keeps reason");
		cepo::bad_result_unwrap e = a;
		e = c;
		check(e.reason() == cepo::bad_result_unwrap::reason_type::is_empty, "assign keeps reason");
	}

	// ---------- 25. recovery after a failed transfer ----------
	std::printf("[25] recovery after failed transfer\n");
	{
		cepo::result<raiser, int> a = cepo::make_result_error<int>(1);
		cepo::result<raiser, int> b = cepo::make_result_value<raiser>();
		a = std::move(b);
		check(a.is_empty() && a.has_exception(), "left empty with exception");
		check(!a.is_ok() && !a.is_error(), "neither ok nor error");

		//empty+exception must be cleanly overwritable
		cepo::result<raiser, int> c = cepo::make_result_error<int>(2);
		a = std::move(c);
		check(a.is_error() && a.unwrap_error() == 2, "recovered by error assign");
		check(!a.has_exception(), "exception cleared after recovery");
	}

	// ---------- 26. destructor balance on overwrite ----------
	std::printf("[26] destructor balance\n");
	{
		check(probe::live == 0, "probe baseline zero");
		{
			cepo::result<probe, int> r = cepo::make_result_value<probe>(1);
			check(probe::live == 1, "one probe alive");
			cepo::result<probe, int> e = cepo::make_result_error<int>(0);
			r = std::move(e);
			check(probe::live == 0, "probe destroyed by error overwrite");
			cepo::result<probe, int> p = cepo::make_result_value<probe>(2);
			r = std::move(p);
			check(probe::live == 1, "second probe alive");
		}
		check(probe::live == 0, "all probes destroyed");
		check(probe::moved >= 2, "moves were performed");
	}

	// ---------- 27. exception_ptr from the slot is rethrowable ----------
	std::printf("[27] rethrow stored exception\n");
	{
		cepo::result<throw_on_copy_ctor, int> r =
			cepo::make_result_value<throw_on_copy_ctor>(throw_on_copy_ctor());
		check(r.has_exception(), "exception recorded");
		std::string msg;
		try { std::rethrow_exception(r.get_exception()); }
		catch (const std::runtime_error& e) { msg = e.what(); }
		catch (...) { msg = "<other>"; }
		check(msg == "ctor boom", "rethrow yields original message");
	}

	// ---------- 28. long string survives the round trip ----------
	std::printf("[28] long string round trip\n");
	{
		std::string big(2048, 'x');
		big[0] = 'A';
		cepo::result<std::string, int> a = cepo::make_result_value<std::string>(big);
		cepo::result<std::string, int> b(std::move(a));
		check(b.is_ok() && b.unwrap_value().size() == 2048, "size preserved");
		check(b.unwrap_value() == big, "content preserved");
	}

	// ---------- 29. ignore_error on both result flavours ----------
	std::printf("[29] ignore_error\n");
	{
		cepo::result<int, int> ok = cepo::make_result_value<int>(1);
		ok.ok([](int&) {}).ignore_error();
		check(ok.is_ok() && ok.unwrap_value() == 1, "value ignore_error keeps state");

		cepo::result<void, int> vok = cepo::make_result_value<void>();
		vok.ok([]() {}).ignore_error();
		check(vok.is_ok(), "void ignore_error keeps state");
	}

	// ---------- 30. global lifetime sanity ----------
	std::printf("[30] lifetime sanity\n");
	{
		check(raiser::live == 0, "raiser instances all destroyed");
		check(probe::live == 0, "probe instances all destroyed");
	}

	std::printf("\nSummary: passed %d, failed %d\n", g_pass, g_fail);
	return g_fail == 0 ? 0 : 1;
}
