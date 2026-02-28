#include "eff.h"
#include <stdio.h>
#include <stdlib.h>

#if defined(_MSC_VER)
#define NOINLINE __declspec(noinline)
#else
#define NOINLINE __attribute__((noinline))
#endif

static void cont_panic(const char *msg)
{
	printf("Panic: %s\n", msg);
	abort();
}

static void get_stack_base(void)
{
#if defined(_MSC_VER) && defined(_M_ARM64)
	cont_stack_base = (char *)_AddressOfReturnAddress();
#elif defined(__aarch64__)
	asm volatile("mov %0, sp" : "=r"(cont_stack_base));
#elif defined(_MSC_VER) && defined(_M_X64)
	cont_stack_base = (char *)_AddressOfReturnAddress();
#elif defined(__x86_64__)
	asm volatile("movq %%rsp, %0" : "=r"(cont_stack_base));
#elif defined(__human68k__)
	asm volatile("move.l %%a7, %0" : "=r"(cont_stack_base));
#else
#error "Unsupported architecture"
#endif
}

static int test_pass;
static int test_fail;

static void check(const char *name, int cond)
{
	if (cond) {
		printf("  PASS: %s\n", name);
		test_pass++;
	} else {
		printf("  FAIL: %s\n", name);
		test_fail++;
	}
}

/* ================================================================
 * Effect tags used in nested handler tests
 * ================================================================ */
enum {
	EFF_YIELD = 1,
	EFF_LOG = 2,
	EFF_RAISE = 3,
	EFF_STATE_GET = 4,
	EFF_STATE_PUT = 5
};

/* ================================================================
 * Test 1: Inner handles YIELD, outer handles LOG
 *
 * The computation performs both YIELD and LOG effects.
 * Inner handler processes YIELD and propagates LOG to outer.
 * ================================================================ */

static eff_handler_t outer1;
static eff_handler_t inner1;
static int inner1_done;
static int yield_count;
static int log_count;

static void computation1(void)
{
	eff_perform(EFF_YIELD, 10);
	eff_perform(EFF_LOG, 100);
	eff_perform(EFF_YIELD, 20);
	eff_perform(EFF_LOG, 200);
	eff_perform(EFF_YIELD, 30);
}

NOINLINE
static void test1(void)
{
	printf("=== Test 1: Inner handles YIELD, outer handles LOG ===\n");

	yield_count = 0;
	log_count = 0;
	inner1_done = 0;

	EFF_HANDLE(outer1) {
		EFF_HANDLE(inner1) {
			computation1();
			inner1_done = 1;
		} EFF_WITH {
			while (!inner1_done) {
				if (eff_tag == EFF_YIELD) {
					yield_count++;
					eff_resume(&eff_k, 0);
				} else {
					/* Propagate unhandled effect to outer */
					eff_reperform(&inner1, eff_tag, eff_arg, &eff_k);
				}
			}
		} EFF_END(inner1);
	} EFF_WITH {
		/* Outer handles LOG */
		if (eff_tag == EFF_LOG) {
			log_count++;
			eff_resume(&eff_k, 0);
		}
	} EFF_END(outer1);

	check("yield_count == 3", yield_count == 3);
	check("log_count == 2", log_count == 2);
}

/* ================================================================
 * Test 2: Inner handles nothing (pure pass-through)
 *
 * Inner handler propagates all effects to outer.
 * Verifies that a "transparent" handler works correctly.
 * ================================================================ */

static eff_handler_t outer2;
static eff_handler_t inner2;
static int inner2_done;
static int outer2_yield_count;
static intptr_t outer2_sum;

static void computation2(void)
{
	eff_perform(EFF_YIELD, 1);
	eff_perform(EFF_YIELD, 2);
	eff_perform(EFF_YIELD, 3);
}

NOINLINE
static void test2(void)
{
	printf("=== Test 2: Inner passes all effects through ===\n");

	outer2_yield_count = 0;
	outer2_sum = 0;
	inner2_done = 0;

	EFF_HANDLE(outer2) {
		EFF_HANDLE(inner2) {
			computation2();
			inner2_done = 1;
		} EFF_WITH {
			while (!inner2_done) {
				/* Pass everything through */
				eff_reperform(&inner2, eff_tag, eff_arg, &eff_k);
			}
		} EFF_END(inner2);
	} EFF_WITH {
		if (eff_tag == EFF_YIELD) {
			outer2_yield_count++;
			outer2_sum += eff_arg;
			eff_resume(&eff_k, 0);
		}
	} EFF_END(outer2);

	check("outer2_yield_count == 3", outer2_yield_count == 3);
	check("outer2_sum == 6", outer2_sum == 6);
}

/* ================================================================
 * Test 3: Three levels of nesting
 *
 * Innermost computation performs YIELD, LOG, and RAISE.
 *   - Level 1 (innermost handler): handles YIELD
 *   - Level 2 (middle handler):    handles LOG
 *   - Level 3 (outermost handler): handles RAISE
 *
 * Effects must propagate through multiple levels.
 * ================================================================ */

static eff_handler_t h3_outer;
static eff_handler_t h3_mid;
static eff_handler_t h3_inner;
static int h3_inner_done;
static int h3_mid_done;
static int h3_yield_count;
static int h3_log_count;
static int h3_raise_caught;

static void computation3(void)
{
	eff_perform(EFF_YIELD, 1);
	eff_perform(EFF_LOG, 10);
	eff_perform(EFF_YIELD, 2);
	eff_perform(EFF_RAISE, 99);
	/* Should not reach here if RAISE aborts */
	eff_perform(EFF_YIELD, 3);
}

NOINLINE
static void test3(void)
{
	printf("=== Test 3: Three-level nesting (YIELD/LOG/RAISE) ===\n");

	h3_yield_count = 0;
	h3_log_count = 0;
	h3_raise_caught = 0;
	h3_inner_done = 0;
	h3_mid_done = 0;

	EFF_HANDLE(h3_outer) {
		EFF_HANDLE(h3_mid) {
			EFF_HANDLE(h3_inner) {
				computation3();
				h3_inner_done = 1;
			} EFF_WITH {
				while (!h3_inner_done) {
					if (eff_tag == EFF_YIELD) {
						h3_yield_count++;
						eff_resume(&eff_k, 0);
					} else {
						eff_reperform(&h3_inner, eff_tag, eff_arg, &eff_k);
					}
				}
			} EFF_END(h3_inner);
			h3_mid_done = 1;
		} EFF_WITH {
			while (!h3_mid_done) {
				if (eff_tag == EFF_LOG) {
					h3_log_count++;
					eff_resume(&eff_k, 0);
				} else {
					eff_reperform(&h3_mid, eff_tag, eff_arg, &eff_k);
				}
			}
		} EFF_END(h3_mid);
	} EFF_WITH {
		/* Outermost: catch RAISE as an exception (don't resume) */
		if (eff_tag == EFF_RAISE) {
			h3_raise_caught = 1;
			/* Don't resume — abort the computation */
		}
	} EFF_END(h3_outer);

	check("h3_yield_count == 2", h3_yield_count == 2);
	check("h3_log_count == 1", h3_log_count == 1);
	check("h3_raise_caught == 1", h3_raise_caught == 1);
}

/* ================================================================
 * Test 4: Handler modifies the value during propagation
 *
 * Inner handler intercepts YIELD, doubles the value, then
 * propagates as YIELD to outer. Tests that values transform
 * correctly across handler boundaries.
 * ================================================================ */

static eff_handler_t outer4;
static eff_handler_t inner4;
static int inner4_done;
static intptr_t collected4[4];
static int collected4_count;

static void computation4(void)
{
	eff_perform(EFF_YIELD, 5);
	eff_perform(EFF_YIELD, 10);
	eff_perform(EFF_YIELD, 15);
}

NOINLINE
static void test4(void)
{
	printf("=== Test 4: Inner transforms values before propagation ===\n");

	collected4_count = 0;
	inner4_done = 0;

	EFF_HANDLE(outer4) {
		EFF_HANDLE(inner4) {
			computation4();
			inner4_done = 1;
		} EFF_WITH {
			while (!inner4_done) {
				/* Double the yield value, then propagate */
				intptr_t doubled = eff_arg * 2;
				eff_reperform(&inner4, eff_tag, doubled, &eff_k);
			}
		} EFF_END(inner4);
	} EFF_WITH {
		if (eff_tag == EFF_YIELD) {
			collected4[collected4_count++] = eff_arg;
			eff_resume(&eff_k, 0);
		}
	} EFF_END(outer4);

	check("collected4_count == 3", collected4_count == 3);
	check("collected4[0] == 10", collected4[0] == 10);
	check("collected4[1] == 20", collected4[1] == 20);
	check("collected4[2] == 30", collected4[2] == 30);
}

/* ================================================================
 * Test 5: Resume value flows back through nested handlers
 *
 * Outer handler provides a value via eff_resume.
 * That value should arrive at the original eff_perform site
 * through eff_reperform.
 * ================================================================ */

static eff_handler_t outer5;
static eff_handler_t inner5;
static int inner5_done;
static intptr_t results5[3];

static void computation5(void)
{
	results5[0] = eff_perform(EFF_LOG, 100);
	results5[1] = eff_perform(EFF_LOG, 200);
	results5[2] = eff_perform(EFF_LOG, 300);
}

NOINLINE
static void test5(void)
{
	printf("=== Test 5: Resume value flows back through reperform ===\n");

	inner5_done = 0;

	EFF_HANDLE(outer5) {
		EFF_HANDLE(inner5) {
			computation5();
			inner5_done = 1;
		} EFF_WITH {
			while (!inner5_done) {
				/* Pass through to outer */
				eff_reperform(&inner5, eff_tag, eff_arg, &eff_k);
			}
		} EFF_END(inner5);
	} EFF_WITH {
		if (eff_tag == EFF_LOG) {
			/* Return the argument negated */
			eff_resume(&eff_k, -eff_arg);
		}
	} EFF_END(outer5);

	check("results5[0] == -100", results5[0] == -100);
	check("results5[1] == -200", results5[1] == -200);
	check("results5[2] == -300", results5[2] == -300);
}

/* ================================================================ */

int main(void)
{
	cont_panic_fn = cont_panic;
	get_stack_base();

	test_pass = 0;
	test_fail = 0;

	test1();
	test2();
	test3();
	test4();
	test5();

	printf("\n--- Results: %d passed, %d failed ---\n",
		test_pass, test_fail);
	return test_fail > 0 ? 1 : 0;
}
