#include "../cont.h"
#include <stdio.h>

#ifdef _MSC_VER
#include <intrin.h>
#endif

void cont_panic(const char *msg)
{
	printf("Panic: %s\n", msg);
}

static void get_stack_base(void)
{
#if defined(__APPLE__) && defined(__aarch64__)
	asm volatile("mov %0, sp" : "=r"(cont_stack_base));
#elif defined(__human68k__)
	asm volatile("move.l %%a7, %0" : "=r"(cont_stack_base));
#elif defined(_MSC_VER) && defined(_WIN64)
	/* Use MSVC intrinsic to get current stack location */
	cont_stack_base = (char *)_AddressOfReturnAddress();
#elif defined(__x86_64__)
	asm volatile("movq %%rsp, %0" : "=r"(cont_stack_base));
#else
#error "Unsupported architecture"
#endif
}

/*
 * Continuation must be stored outside the stack area being captured.
 */
static cont_t cont;

#ifdef _MSC_VER
__declspec(noinline)
#else
__attribute__((noinline))
#endif
static void run_test(void)
{
	cont_init(&cont);
	printf("cont initialized.\n");
	if (cont_save(&cont) == 0) {
		printf("cont saved.\n");
		cont_resume(&cont);
		printf("We should not reach here.\n");
	}
	printf("cont resumed.\n");
	cont_term(&cont);
	printf("cont terminated.\n");
}

int main(void)
{
	cont_panic_fn = cont_panic;
	get_stack_base();
	run_test();
	return 0;
}
