#include "eff.h"
#include <stdio.h>

#if defined(_MSC_VER)
#include <intrin.h>
#endif

static void cont_panic(const char *msg)
{
	printf("Panic: %s\n", msg);
}

static void get_stack_base(void)
{
#if defined(__aarch64__)
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

/* ================================================================
 * Test 1: Exception effect (simplest perform + resume)
 * ================================================================ */

enum { EFF_RAISE = 1 };

static intptr_t safe_div(intptr_t a, intptr_t b)
{
	if (b == 0)
		return eff_perform(EFF_RAISE, 0);
	return a / b;
}

static eff_handler_t h1;

#ifdef _MSC_VER
__declspec(noinline)
#else
__attribute__((noinline))
#endif
static void test1(void)
{
	printf("=== Test 1: Exception effect ===\n");

	intptr_t result;

	EFF_HANDLE(h1) {
		result = safe_div(10, 0);
	} EFF_WITH {
		printf("Handler caught EFF_RAISE, returning -1\n");
		eff_resume(&eff_k, -1);
	} EFF_END(h1);

	printf("safe_div returned: %ld\n", (long)result);
}

/* ================================================================
 * Test 2: Generator (yield) effect — Fibonacci
 * ================================================================ */

enum { EFF_YIELD = 2 };

static void fib_generator(void)
{
	intptr_t a = 0, b = 1;
	for (int i = 0; i < 8; i++) {
		eff_perform(EFF_YIELD, a);
		intptr_t next = a + b;
		a = b;
		b = next;
	}
}

static eff_handler_t h2;
static int gen_done;

#ifdef _MSC_VER
__declspec(noinline)
#else
__attribute__((noinline))
#endif
static void test2(void)
{
	printf("=== Test 2: Generator effect ===\n");

	gen_done = 0;

	EFF_HANDLE(h2) {
		fib_generator();
		gen_done = 1;
	} EFF_WITH {
		while (!gen_done) {
			printf("  yielded: %ld\n", (long)eff_arg);
			eff_resume(&eff_k, 0);
		}
	} EFF_END(h2);

	printf("Generator complete.\n");
}

/* ================================================================
 * Test 3: State effect (get/put)
 * ================================================================ */

enum { EFF_STATE_GET = 3,
	EFF_STATE_PUT = 4 };

static intptr_t state_get(void)
{
	return eff_perform(EFF_STATE_GET, 0);
}

static void state_put(intptr_t v)
{
	eff_perform(EFF_STATE_PUT, v);
}

static intptr_t state_computation(void)
{
	intptr_t v = state_get();
	printf("  got state: %ld\n", (long)v);
	state_put(v + 10);

	v = state_get();
	printf("  got state: %ld\n", (long)v);
	state_put(v + 10);

	v = state_get();
	printf("  got state: %ld\n", (long)v);
	return v;
}

static eff_handler_t h3;
static intptr_t state_val;
static intptr_t comp_result;
static int comp_done;

#ifdef _MSC_VER
__declspec(noinline)
#else
__attribute__((noinline))
#endif
static void test3(void)
{
	printf("=== Test 3: State effect ===\n");

	state_val = 42;
	comp_done = 0;

	EFF_HANDLE(h3) {
		comp_result = state_computation();
		comp_done = 1;
	} EFF_WITH {
		while (!comp_done) {
			switch (eff_tag) {
			case EFF_STATE_GET:
				eff_resume(&eff_k, state_val);
				break;
			case EFF_STATE_PUT:
				state_val = eff_arg;
				eff_resume(&eff_k, 0);
				break;
			}
		}
	} EFF_END(h3);

	printf("Final state: %ld, result: %ld\n", (long)state_val, (long)comp_result);
}

/* ================================================================ */

int main(void)
{
	cont_panic_fn = cont_panic;
	get_stack_base();

	test1();
	test2();
	test3();

	return 0;
}
