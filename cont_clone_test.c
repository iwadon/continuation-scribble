/* cont_clone_test.c : multi-shot continuation test */
#include "cont.h"
#include <stdio.h>

#if defined(_MSC_VER)
#include <intrin.h>
#endif

static void cont_panic(const char *msg) { printf("Panic: %s\n", msg); }

static void get_stack_base(void)
{
#if defined(__aarch64__)
	asm volatile("mov %0, sp" : "=r"(cont_stack_base));
#elif defined(__human68k__)
	asm volatile("move.l %%a7, %0" : "=r"(cont_stack_base));
#elif defined(_MSC_VER) && defined(_M_X64)
	cont_stack_base = (char *)_AddressOfReturnAddress();
#elif defined(__x86_64__)
	asm volatile("movq %%rsp, %0" : "=r"(cont_stack_base));
#else
#error "Unsupported architecture"
#endif
}

/*
 * Continuations must be stored outside the stack area being captured,
 * otherwise resuming will corrupt the continuation structures themselves.
 */
static cont_t base, c1, c2;
static int resume_count = 0;

/*
 * Run the actual test in a separate function so that the stack frame
 * is properly captured between cont_stack_base and the cont_save point.
 */
#ifdef _MSC_VER
__declspec(noinline)
#else
__attribute__((noinline))
#endif
static void run_test(void)
{
	cont_init(&base);
	cont_init(&c1);
	cont_init(&c2);
	printf("cont initialized.\n");

	if (cont_save(&base) == 0) {
		printf("cont saved once. stack_size=%zu\n", base.stack_size);
		fflush(stdout);
		cont_clone(&c1, &base);
		cont_clone(&c2, &base);
		printf("resume c1\n");
		fflush(stdout);
		cont_resume(&c1);
		printf("We should not reach here.\n");
	}

	resume_count++;
	printf("resumed (count=%d)\n", resume_count);

	if (resume_count == 1) {
		printf("resume c2\n");
		cont_resume(&c2);
		printf("We should not reach here too.\n");
	}

	printf("test completed successfully.\n");
	cont_term(&c2);
	cont_term(&c1);
	cont_term(&base);
}

int main(void)
{
	cont_panic_fn = cont_panic;

	/* Get stack base at the top of main, before any function calls */
	get_stack_base();

	/* Run test in nested function so its stack is captured */
	run_test();

	return 0;
}
