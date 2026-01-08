#if !defined(CONT_H_INCLUDED)
#define CONT_H_INCLUDED

#include <stddef.h>

/* Architecture-specific context type */
#if defined(__APPLE__) && defined(__aarch64__)
#include "arm64_ctx.h"
typedef arm64_ctx_t cont_ctx_t;
#elif defined(__human68k__)
#include "m68k_ctx.h"
typedef m68k_ctx_t cont_ctx_t;
#elif defined(__x86_64__)
#include "x86_64/x86_64_ctx.h"
typedef x86_64_ctx_t cont_ctx_t;
#else
#error "Unsupported architecture for continuation"
#endif

typedef struct {
	cont_ctx_t ctx;
	void *stack_image;
	size_t stack_size;
	size_t stack_capacity;
} cont_t;

extern char *cont_stack_base;

typedef void (*cont_panic_fn_t)(const char *msg);
extern cont_panic_fn_t cont_panic_fn;

extern void cont_init(cont_t *cont);
extern void cont_term(cont_t *cont);
extern int cont_save(cont_t *cont);
__attribute__((noreturn)) extern void cont_resume(const cont_t *cont);
extern void cont_clone(cont_t *dst, const cont_t *src);

#endif // !defined(CONT_H_INCLUDED)
