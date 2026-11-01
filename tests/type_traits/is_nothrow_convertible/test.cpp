#include <cstddef>
#include <cstdio>
#include <memory>
#include "cepo/type_traits/is_nothrow_convertible.hpp"

#define TEST(from, to, expect) \
	put_bool(#from, #to, cepo::is_nothrow_convertible<from, to>::value, expect)

int unexpected = 0;

void put_bool(const char* from, const char* to, bool fact, bool expect) noexcept
{
	std::printf(
		"item:\nfrom %s to %s\nvalue: %s\nresult: %s\n\n",
		from, to, fact ? "true" : "false",
		expect == fact ? "good" : "bad"
	);
	unexpected += not (fact == expect);
}

struct nothrow_ctor_from_int
{
	nothrow_ctor_from_int(int) noexcept {}
};

struct throwing_ctor_from_int
{
	throwing_ctor_from_int(int) {}
};

struct explicit_nothrow_ctor
{
	explicit explicit_nothrow_ctor(int) noexcept {}
};

struct explicit_throwing_ctor
{
	explicit explicit_throwing_ctor(int) {}
};

struct multi_arg_ctor
{
	multi_arg_ctor(int, int = 0) noexcept {}
};

struct nothrow_conv_op
{
	operator int() const noexcept { return 0; }
};

struct throwing_conv_op
{
	operator int() const { return 0; }
};

struct explicit_nothrow_conv_op
{
	explicit operator int() const noexcept { return 0; }
};

struct explicit_throwing_conv_op
{
	explicit operator int() const { return 0; }
};

struct implicit_bool
{
	operator bool() const noexcept { return true; }
};

struct explicit_bool
{
	explicit operator bool() const noexcept { return true; }
};

struct nothrow_to_int_ref
{
	operator int&() const noexcept
	{
		static int x = 0;
		return x;
	}
};

struct throwing_to_int_ref
{
	operator int&() const
	{
		static int x = 0;
		return x;
	}
};

struct move_noexcept_copy_throw
{
	move_noexcept_copy_throw(int) noexcept {}
	move_noexcept_copy_throw(move_noexcept_copy_throw const&) {}
	move_noexcept_copy_throw(move_noexcept_copy_throw&&) noexcept = default;
};

struct move_throw_copy_noexcept
{
	move_throw_copy_noexcept(int) noexcept {}
	move_throw_copy_noexcept(move_throw_copy_noexcept const&) noexcept = default;
	move_throw_copy_noexcept(move_throw_copy_noexcept&&) {}
};

struct move_throw_copy_throw
{
	move_throw_copy_throw(int) noexcept {}
	move_throw_copy_throw(move_throw_copy_throw const&) {}
	move_throw_copy_throw(move_throw_copy_throw&&) {}
};

struct nothrow_move_and_copy
{
	nothrow_move_and_copy(int) noexcept {}
	nothrow_move_and_copy(nothrow_move_and_copy const&) noexcept = default;
	nothrow_move_and_copy(nothrow_move_and_copy&&) noexcept = default;
};

struct non_movable
{
	non_movable() noexcept = default;
	non_movable(int) noexcept {}
	non_movable(non_movable const&) = delete;
	non_movable(non_movable&&) = delete;
};

struct base {};
struct derived : base {};

struct abstract
{
	virtual void f() = 0;
};

struct concrete : abstract
{
	void f() override {}
};

enum unscoped_enum { eu0 };
enum class scoped_enum { es0 };

struct mem_class
{
	int member;
	void method();
};

struct priv_derived : private base {};
struct prot_derived : protected base {};

struct mi_a {};
struct mi_b {};
struct mi_c : mi_a, mi_b {};

struct vbase {};
struct vl : virtual vbase {};
struct vr : virtual vbase {};
struct vdiamond : vl, vr {};

auto main() -> int
{
	TEST(int, int, true);
	TEST(int, long, true);
	TEST(int, unsigned, true);
	TEST(int, float, true);
	TEST(int, double, true);
	TEST(int, long double, true);
	TEST(long double, int, true);
	TEST(double, int, true);
	TEST(char, int, true);
	TEST(bool, int, true);
	TEST(int, bool, true);
	TEST(int, char, true);
	TEST(char, bool, true);
	TEST(bool, char, true);
	TEST(unsigned, int, true);

	TEST(int*, int*, true);
	TEST(int*, void*, true);
	TEST(int*, const void*, true);
	TEST(int*, volatile void*, true);
	TEST(int*, const int*, true);
	TEST(int*, volatile int*, true);
	TEST(const int*, int*, false);
	TEST(volatile int*, int*, false);
	TEST(const int*, void*, false);
	TEST(const int*, const void*, true);
	TEST(void*, int*, false);
	TEST(void*, const void*, true);
	TEST(void*, volatile void*, true);
	TEST(const void*, void*, false);
	TEST(int*, bool, true);
	TEST(double*, bool, true);
	TEST(int*, char*, false);
	TEST(char*, int*, false);
	TEST(int*, long*, false);
	TEST(int**, void*, true);
	TEST(int**, int*, false);
	TEST(int, int*, false);
	TEST(int*, int, false);

	TEST(std::nullptr_t, std::nullptr_t, true);
	TEST(std::nullptr_t const&, std::nullptr_t, true);
	TEST(std::nullptr_t, std::nullptr_t const&, true);
	TEST(std::nullptr_t, std::nullptr_t&, false);
	TEST(std::nullptr_t, int*, true);
	TEST(std::nullptr_t, void*, true);
	TEST(std::nullptr_t, char*, true);
	TEST(std::nullptr_t, bool, false);
	TEST(std::nullptr_t, int, false);
	TEST(int*, std::nullptr_t, false);
	TEST(int, std::nullptr_t, false);
	TEST(bool, std::nullptr_t, false);

	TEST(int, int&, false);
	TEST(int, int const&, true);
	TEST(int, int&&, true);
	TEST(int, int const&&, true);
	TEST(int&, int, true);
	TEST(int&, int&, true);
	TEST(int&, int const&, true);
	TEST(int&, int&&, false);
	TEST(int const&, int&, false);
	TEST(int const&, int const&, true);
	TEST(int const&, int, true);
	TEST(int&&, int&, false);
	TEST(int&&, int&&, true);
	TEST(int&&, int const&, true);
	TEST(int&&, int, true);
	TEST(int const&&, int const&, true);
	TEST(int const&&, int&&, false);

	TEST(void, void, true);
	TEST(void, int, false);
	TEST(void, int*, false);

	TEST(int, nothrow_ctor_from_int, true);
	TEST(int, throwing_ctor_from_int, false);
	TEST(int, explicit_nothrow_ctor, false);
	TEST(int, explicit_throwing_ctor, false);
	TEST(int, multi_arg_ctor, true);
	TEST(int, nothrow_move_and_copy, true);

	TEST(nothrow_conv_op, int, true);
	TEST(nothrow_conv_op, double, true);
	TEST(nothrow_conv_op, bool, true);
	TEST(nothrow_conv_op, long, true);
	TEST(throwing_conv_op, int, false);
	TEST(throwing_conv_op, double, false);
	TEST(explicit_nothrow_conv_op, int, false);
	TEST(explicit_throwing_conv_op, int, false);
	TEST(implicit_bool, bool, true);
	TEST(implicit_bool, int, true);
	TEST(explicit_bool, bool, false);

	TEST(nothrow_to_int_ref, int&, true);
	TEST(nothrow_to_int_ref, int const&, true);
	TEST(nothrow_to_int_ref, int, true);
	TEST(throwing_to_int_ref, int&, false);
	TEST(throwing_to_int_ref, int, false);

	TEST(move_noexcept_copy_throw, move_noexcept_copy_throw, true);
	TEST(move_throw_copy_noexcept, move_throw_copy_noexcept, false);
	TEST(move_throw_copy_throw, move_throw_copy_throw, false);
	TEST(nothrow_move_and_copy, nothrow_move_and_copy, true);
	TEST(non_movable, non_movable, false);

	TEST(move_noexcept_copy_throw&, move_noexcept_copy_throw, false);
	TEST(move_throw_copy_noexcept&, move_throw_copy_noexcept, true);
	TEST(non_movable&, non_movable, false);

	TEST(move_throw_copy_throw, move_throw_copy_throw const&, true);
	TEST(move_throw_copy_throw, move_throw_copy_throw&&, true);
	TEST(move_throw_copy_throw, move_throw_copy_throw&, false);
	TEST(move_throw_copy_throw&, move_throw_copy_throw&, true);

	TEST(int, move_noexcept_copy_throw, true);
	TEST(int, nothrow_move_and_copy, true);

	TEST(derived*, base*, true);
	TEST(base*, derived*, false);
	TEST(derived*, void*, true);
	TEST(derived*, bool, true);
	TEST(derived*, const base*, true);
	TEST(const derived*, base*, false);
	TEST(const derived*, const base*, true);
	TEST(derived&, base&, true);
	TEST(base&, derived&, false);
	TEST(derived&, base const&, true);
	TEST(const derived&, base&, false);
	TEST(derived&&, base&&, true);
	TEST(derived&&, base&, false);
	TEST(derived&&, base const&, true);
	TEST(derived, base, true);
	TEST(base, derived, false);

	TEST(int, abstract, false);
	TEST(concrete, abstract, false);
	TEST(concrete&, abstract&, true);
	TEST(concrete*, abstract*, true);
	TEST(abstract*, concrete*, false);
	TEST(abstract&, concrete&, false);
	TEST(concrete, concrete, true);
	TEST(concrete, concrete&, false);
	TEST(concrete, concrete const&, true);

	TEST(unscoped_enum, int, true);
	TEST(unscoped_enum, long, true);
	TEST(unscoped_enum, bool, true);
	TEST(unscoped_enum, double, true);
	TEST(scoped_enum, int, false);
	TEST(scoped_enum, bool, false);
	TEST(int, unscoped_enum, false);
	TEST(int, scoped_enum, false);
	TEST(unscoped_enum, scoped_enum, false);
	TEST(scoped_enum, unscoped_enum, false);

	TEST(int[3], int*, true);
	TEST(int[3], const int*, true);
	TEST(int[3], void*, true);
	TEST(int[3], bool, true);
	TEST(int[3], int const(&)[3], true);
	TEST(int[3], int(&)[3], false);
	TEST(int[3], int[3], false);
	TEST(int(&)[3], int*, true);
	TEST(int(&)[3], int(&)[3], true);
	TEST(int(&)[3], int, false);
	TEST(const int[3], int*, false);
	TEST(const int[3], const int*, true);

	TEST(void(), void(*)(), true);
	TEST(void(&)(), void(*)(), true);
	TEST(void(*)(), void(*)(), true);
	TEST(void(*)(), bool, true);
	TEST(void(*)(), void*, false);
	TEST(void(*)(), void(), false);
	TEST(void(), void(), false);
	TEST(void(), bool, true);
	TEST(void(), int, false);
	TEST(int(*)(), bool, true);
	TEST(int(*)(), void*, false);

	TEST(int mem_class::*, bool, true);
	TEST(int mem_class::*, void*, false);
	TEST(void (mem_class::*)(), bool, true);
	TEST(void (mem_class::*)(), void*, false);
	TEST(int mem_class::*, int*, false);

	TEST(std::unique_ptr<int>, std::unique_ptr<int>, true);
	TEST(std::unique_ptr<int>, std::unique_ptr<const int>, true);
	TEST(std::unique_ptr<int>, bool, false);
	TEST(std::unique_ptr<int>, int*, false);
	TEST(std::unique_ptr<int>&, std::unique_ptr<int>, false);


	TEST(mi_c*, mi_a*, true);
	TEST(mi_c*, mi_b*, true);
	TEST(mi_a*, mi_b*, false);
	TEST(mi_a*, mi_c*, false);

	TEST(priv_derived*, base*, false);
	TEST(prot_derived*, base*, false);

	TEST(vdiamond*, vbase*, true);
	TEST(vdiamond*, vl*, true);
	TEST(vdiamond*, vr*, true);

	TEST(derived, base&, false);
	TEST(derived, base const&, true);
	TEST(derived, base&&, true);

	TEST(std::nullptr_t, void(*)(), true);
	TEST(std::nullptr_t, int mem_class::*, true);

	TEST(int&&, int const&&, true);
	TEST(int&, int const&&, false);
	TEST(int, int volatile&, false);

	TEST(non_movable&, non_movable&, true);

	TEST(volatile int*, bool, true);
	TEST(const void*, bool, true);

	TEST(scoped_enum, double, false);
	TEST(unscoped_enum, unsigned long long, true);

	std::printf("unexpected: %d\n", unexpected);
	return unexpected != 0;
}
