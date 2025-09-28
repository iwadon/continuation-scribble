#include "fiber.h"
#if defined(__APPLE__) && defined(__aarch64__)
#include "arm64_ctx.h"
#elif defined(__human68k__)
#include "m68k_ctx.h"
#elif defined(__x86_64__)
#include "x86_64/x86_64_ctx.h"
#endif
#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct fiber {
#if defined(__APPLE__) && defined(__aarch64__)
	arm64_ctx_t ctx;
#elif defined(__human68k__)
	m68k_ctx_t ctx;
#elif defined(__x86_64__)
	x86_64_ctx_t ctx;
#endif
	void *stack;
	size_t stack_size;
	void (*entry)(void *);
	void *arg;
	fib_state_t state;
};

#if defined(__APPLE__) && defined(__aarch64__)
#define ctx_save(c) arm64_ctx_save(c)
#define ctx_resume(c) arm64_ctx_resume(c)
#elif defined(__human68k__)
#define ctx_save(c) m68k_ctx_save(c)
#define ctx_resume(c) m68k_ctx_resume(c)
#elif defined(__x86_64__)
#define ctx_save(c) x86_64_ctx_save(c)
#define ctx_resume(c) x86_64_ctx_resume(c)
#else
#error "Unsupported architecture"
#endif

/* Globals */
static fiber_t *g_main = 0;
static fiber_t *g_current = 0;

/* ASM trampoline (first entry) */
void fiber_trampoline(void) __attribute__((noreturn));

/* --- internals --- */
#if defined(__APPLE__) && defined(__aarch64__)
static void fiber_raw_jump(const arm64_ctx_t *c) __attribute__((noreturn));
static void fiber_raw_jump(const arm64_ctx_t *c)
{
	arm64_ctx_resume(c);
	__builtin_abort();
}
#elif defined(__human68k__)
static void fiber_raw_jump(const m68k_ctx_t *c) __attribute__((noreturn));
static void fiber_raw_jump(const m68k_ctx_t *c)
{
	m68k_ctx_resume(c);
	__builtin_abort();
}
#elif defined(__x86_64__)
static void fiber_raw_jump(const x86_64_ctx_t *c) __attribute__((noreturn));
static void fiber_raw_jump(const x86_64_ctx_t *c)
{
	x86_64_ctx_resume(c);
	__builtin_abort();
}
#endif

static void fiber_swap_to(fiber_t *to)
{
	fiber_t *from = g_current;
	assert(to);
	if (!from) {
		/* first ever switch from main */
		from = g_main;
	}
	if (ctx_save(&from->ctx) == 0) {
		g_current = to;			  /* visible on the other side */
		fiber_raw_jump(&to->ctx); /* noreturn */
		abort();
	}
	/* resumed here (on "to" side) */
}

/* Exported helpers */
fiber_t *fiber_self(void) { return g_current ? g_current : g_main; }
fiber_t *fiber_main(void) { return g_main; }

int fiber_main_init(fiber_t **out_main)
{
	if (!out_main) {
		return -1;
	}
	if (g_main) {
		*out_main = g_main;
		return 0;
	}
	fiber_t *m = (fiber_t *)calloc(1, sizeof(*m));
	if (!m) {
		return -2;
	}
	m->state = FIB_RUNNING;
	g_main = g_current = m;
	*out_main = m;
	return 0;
}

int fiber_create(fiber_t **out, size_t stack_size, void (*entry)(void *), void *arg)
{
	if (!out || !entry || stack_size < 1024) {
		return -1;
	}
	fiber_t *fb = (fiber_t *)calloc(1, sizeof(*fb));
	if (!fb) {
		return -2;
	}
	fb->stack_size = (stack_size + 15u) & ~15u;
	fb->stack = malloc(fb->stack_size);
	if (!fb->stack) {
		free(fb);
		return -3;
	}
	fb->entry = entry;
	fb->arg = arg;
	fb->state = FIB_READY;

	/* Setup initial context: SP at top of stack, PC=fiber_trampoline. */
	memset(&fb->ctx, 0, sizeof(fb->ctx));
#if defined(__APPLE__) && defined(__aarch64__)
	fb->ctx.sp = (uint64_t)(unsigned long)(fb->stack + fb->stack_size);
	fb->ctx.lr = (uint64_t)(unsigned long)fiber_trampoline;
	/* Pass entry/arg via X2/X3 (safe w.r.t. arm64_ctx_resume) */
	fb->ctx.x[2] = (uint64_t)(unsigned long)entry;
	fb->ctx.x[3] = (uint64_t)(unsigned long)arg;
#elif defined(__human68k__)
	fb->ctx.ret = (uint32_t)(void *)fiber_trampoline;
	fb->ctx.a[7] = (uint32_t)((unsigned long)fb->stack + fb->stack_size);
	/* Pass entry/arg via A2/D2 (safe w.r.t. m68k_ctx_resume) */
	fb->ctx.a[2] = (uint32_t)(unsigned long)entry;
	fb->ctx.d[2] = (uint32_t)(unsigned long)arg;
#elif defined(__x86_64__)
    fb->ctx.rip = (uint64_t)fiber_trampoline;
    fb->ctx.rsp = (uint64_t)((uint8_t *)fb->stack + fb->stack_size);
    // Align stack to 16 bytes, then push a dummy return address for 16-byte alignment before call
    fb->ctx.rsp -= 8; 
    // Pass entry and arg via callee-saved registers
    fb->ctx.rbx = (uint64_t)entry;
    fb->ctx.r12 = (uint64_t)arg;
#endif

	*out = fb;
	return 0;
}

void fiber_destroy(fiber_t *fb)
{
	if (!fb) {
		return;
	}
	assert(fb != g_current); /* must not destroy the running fiber */
	free(fb->stack);
	free(fb);
}

fib_state_t fiber_state(const fiber_t *fb) { return fb ? fb->state : FIB_DONE; }
int fiber_is_done(const fiber_t *fb) { return fb ? (fb->state == FIB_DONE) : 1; }

void fiber_yield(void) { fiber_yield_to(g_main); }
void fiber_yield_to(fiber_t *to) { fiber_swap_to(to); }

void fiber_mark_done_and_return(void)
{
	fiber_t *self = fiber_self();
	self->state = FIB_DONE;
	fiber_yield_to(g_main); /* never returns */
	abort();
}
