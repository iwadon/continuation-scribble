#if !defined(CONT_H_INCLUDED)
#define CONT_H_INCLUDED

#include "m68k_ctx.h"
#include <stddef.h>

typedef struct {
	m68k_ctx_t ctx;
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
