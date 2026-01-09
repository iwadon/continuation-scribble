#include "cont.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Architecture-specific macros for context operations */
#if defined(__aarch64__)
#define cont_ctx_save(c) arm64_ctx_save(c)
#define cont_ctx_resume(c) arm64_ctx_resume(c)
#define CONT_GET_SP(ctx) ((char *)(uintptr_t)(ctx)->sp)
#define CONT_SET_SP(ctx, val) ((ctx)->sp = (uint64_t)(uintptr_t)(val))
#elif defined(__human68k__)
#define cont_ctx_save(c) m68k_ctx_save(c)
#define cont_ctx_resume(c) m68k_ctx_resume(c)
#define CONT_GET_SP(ctx) ((char *)(uintptr_t)(ctx)->a[7])
#define CONT_SET_SP(ctx, val) ((ctx)->a[7] = (uint32_t)(uintptr_t)(val))
#elif defined(_WIN64) || defined(__x86_64__)
#define cont_ctx_save(c) x86_64_ctx_save(c)
#define cont_ctx_resume(c) x86_64_ctx_resume(c)
#define CONT_GET_SP(ctx) ((char *)(uintptr_t)(ctx)->rsp)
#define CONT_SET_SP(ctx, val) ((ctx)->rsp = (uint64_t)(uintptr_t)(val))
#endif

char *cont_stack_base = NULL;
cont_panic_fn_t cont_panic_fn = NULL;

void cont_init(cont_t *cont)
{
	if (cont == NULL) {
		if (cont_panic_fn != NULL) {
			cont_panic_fn("cont_init: invalid argument");
		}
		abort();
	}
	memset(&cont->ctx, 0, sizeof cont->ctx);
	cont->stack_image = NULL;
	cont->stack_size = 0;
	cont->stack_capacity = 0;
}

void cont_term(cont_t *cont)
{
	if (cont == NULL) {
		if (cont_panic_fn != NULL) {
			cont_panic_fn("cont_term: invalid argument");
		}
		abort();
	}
	if (cont->stack_image != NULL) {
		free(cont->stack_image);
		cont->stack_image = NULL;
	}
	cont->stack_size = 0;
	cont->stack_capacity = 0;
}

int cont_save(cont_t *cont)
{
	if (cont == NULL) {
		if (cont_panic_fn != NULL) {
			cont_panic_fn("cont_save: invalid argument");
		}
		abort();
	}
	if (cont_ctx_save(&cont->ctx) != 0) {
		return 1;
	}
	char *sp = CONT_GET_SP(&cont->ctx);
	size_t need = (size_t)(cont_stack_base - sp);
	if (cont->stack_capacity < need) {
		size_t new_capacity = cont->stack_capacity > 0 ? cont->stack_capacity : 1024;
		while (new_capacity < need) {
			new_capacity <<= 1;
		}
		void *p = realloc(cont->stack_image, new_capacity);
		if (p == NULL) {
			if (cont_panic_fn != NULL) {
				cont_panic_fn("cont_save: out of memory");
			}
			abort();
		}
		cont->stack_image = p;
		cont->stack_capacity = new_capacity;
	}
	memcpy(cont->stack_image, sp, need);
	cont->stack_size = need;
	return 0;
}

/*
 * cont_resume_internal: actual resume logic
 * This function must be called after moving SP below the restoration area.
 * Not static because it's called from inline assembly (or MASM trampoline).
 */
#ifdef _MSC_VER
__declspec(noinline) void cont_resume_internal(const cont_t *cont)
#else
void __attribute__((noinline)) cont_resume_internal(const cont_t *cont)
#endif
{
	/* Restore stack image to original location */
	char *sp = CONT_GET_SP(&cont->ctx);
	memcpy(sp, cont->stack_image, cont->stack_size);
	cont_ctx_resume(&cont->ctx);
}

#ifdef _WIN64
/* External trampoline function defined in cont_trampoline.asm for Windows */
extern void cont_resume_trampoline(char *safe_sp, const cont_t *cont,
	void (*fn)(const cont_t *));
#endif

void cont_resume(const cont_t *cont)
{
	if (cont == NULL || cont->stack_image == NULL || cont->stack_size == 0) {
		if (cont_panic_fn != NULL) {
			cont_panic_fn("cont_resume: invalid argument");
		}
		abort();
	}
	/*
	 * Move SP below the area to be restored to avoid corrupting
	 * our own stack frame during memcpy.
	 * The target SP is cont->ctx's SP, and we need space below it.
	 */
	char *target_sp = CONT_GET_SP(&cont->ctx);
	size_t guard = cont->stack_size + 4096; /* extra space for safety */
	char *safe_sp = target_sp - guard;

{
	void (*fn)(const cont_t *) = cont_resume_internal;
#if defined(__aarch64__)
	/* Move SP down and call internal function */
	asm volatile(
		"mov sp, %0\n\t"
		"mov x0, %1\n\t"
		"blr %2\n\t"
		:
		: "r"(safe_sp), "r"(cont), "r"(fn)
		: "memory");
#elif defined(__human68k__)
	asm volatile(
		"move.l %0, %%sp\n\t"
		"move.l %1, -(%%sp)\n\t"
		"jsr (%2)\n\t"
		:
		: "r"(safe_sp), "r"(cont), "a"(fn)
		: "memory");
#elif defined(_WIN64)
	/* Use external MASM trampoline since MSVC doesn't support inline asm */
	cont_resume_trampoline(safe_sp, cont, fn);
#elif defined(__x86_64__)
	asm volatile(
		"movq %0, %%rsp\n\t"
		"movq %1, %%rdi\n\t"
		"callq *%2\n\t"
		:
		: "r"(safe_sp), "r"(cont), "r"(fn)
		: "memory");
#endif
}
#ifdef _MSC_VER
	/* unreachable */
	for (;;) {}
#else
	__builtin_unreachable();
#endif
}

void cont_clone(cont_t *dst, const cont_t *src)
{
	if (dst == NULL || src == NULL || src->stack_image == NULL || src->stack_size == 0) {
		if (cont_panic_fn != NULL) {
			cont_panic_fn("cont_clone: invalid argument");
		}
		abort();
	}
	dst->ctx = src->ctx;
	dst->stack_size = src->stack_size;
	if (dst->stack_capacity < src->stack_size) {
		void *p = realloc(dst->stack_image, src->stack_size);
		if (p == NULL) {
			if (cont_panic_fn != NULL) {
				cont_panic_fn("cont_clone: out of memory");
			}
			abort();
		}
		dst->stack_image = p;
		dst->stack_capacity = src->stack_size;
	}
	memcpy(dst->stack_image, src->stack_image, src->stack_size);
}
