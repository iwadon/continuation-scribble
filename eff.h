#if !defined(EFF_H_INCLUDED)
#define EFF_H_INCLUDED

#include "cont.h"
#include <stdint.h>

#if defined(_MSC_VER)
#include <intrin.h>
#endif

/* --- Algebraic Effects for C using delimited continuations --- */

/* Handler frame: linked list forming a handler stack */
typedef struct eff_handler_t {
	struct eff_handler_t *prev;
	cont_t escape;
	char *saved_stack_base;
} eff_handler_t;

/* Captured continuation from a perform site */
typedef struct {
	cont_t cont;
} eff_cont_t;

/* Global state (single-threaded) */
extern eff_handler_t *eff_handler_stack;
extern int eff_tag;
extern intptr_t eff_arg;
extern eff_cont_t eff_k;
extern intptr_t eff_resume_val;

/* Get current stack pointer (architecture-specific macro).
 * Must be a macro, not a function, to read the caller's SP even at -O0. */
#if defined(_MSC_VER) && defined(_M_ARM64)
#define EFF_GET_SP(dst) ((dst) = (char *)_AddressOfReturnAddress())
#elif defined(__aarch64__)
#define EFF_GET_SP(dst) asm volatile("mov %0, sp" : "=r"(dst))
#elif defined(_MSC_VER) && defined(_M_X64)
#define EFF_GET_SP(dst) ((dst) = (char *)_AddressOfReturnAddress())
#elif defined(__x86_64__)
#define EFF_GET_SP(dst) asm volatile("movq %%rsp, %0" : "=r"(dst))
#elif defined(__human68k__)
#define EFF_GET_SP(dst) asm volatile("move.l %%a7, %0" : "=r"(dst))
#else
#error "Unsupported architecture for EFF_GET_SP"
#endif

/*
 * eff_perform(tag, arg) — suspend computation, jump to handler.
 * Returns the value passed by eff_resume().
 */
intptr_t eff_perform(int tag, intptr_t arg);

/*
 * eff_resume(k, val) — resume a captured continuation with a value.
 * Does not return (noreturn).
 */
#if defined(_MSC_VER)
__declspec(noreturn)
#else
__attribute__((noreturn))
#endif
void eff_resume(eff_cont_t *k, intptr_t val);

/*
 * eff_reperform(h, tag, arg, k) — propagate an effect to the outer handler.
 *
 * Call this from a handler (EFF_WITH block) when the current handler
 * does not handle the given tag.  It temporarily pops the current
 * handler, performs the effect to the next handler up the stack,
 * and on resume restores the handler stack and resumes the original
 * computation captured in k.
 *
 * h   — pointer to the current handler (the one calling reperform)
 * tag — effect tag to propagate
 * arg — effect argument to propagate
 * k   — pointer to the captured continuation from the computation
 */
void eff_reperform(eff_handler_t *h, int tag, intptr_t arg, eff_cont_t *k);

/*
 * eff_handle_end(h) — pop handler, restore cont_stack_base, clean up.
 */
void eff_handle_end(eff_handler_t *h);

/*
 * EFF_HANDLE / EFF_WITH / EFF_END macros
 *
 * Usage:
 *   static eff_handler_t h;
 *   EFF_HANDLE(h) {
 *       // computation that may perform effects
 *   } EFF_WITH {
 *       // handler: eff_tag, eff_arg, eff_k are available
 *   } EFF_END(h);
 *
 * Implementation:
 *   EFF_HANDLE sets up the handler frame and saves an escape continuation.
 *   cont_save returns 0 on initial save (run computation),
 *   and 1 when resumed via perform (run handler).
 */
#define EFF_HANDLE(h)                                   \
	do {                                            \
		cont_init(&(h).escape);                 \
		(h).saved_stack_base = cont_stack_base; \
		(h).prev = eff_handler_stack;           \
		eff_handler_stack = &(h);               \
		EFF_GET_SP(cont_stack_base);            \
		if (cont_save(&(h).escape) == 0) {

#define EFF_WITH                       \
		} else { /* handler */

#define EFF_END(h)                 \
		}                  \
		eff_handle_end(&(h));  \
	} while (0)

#endif /* !defined(EFF_H_INCLUDED) */
