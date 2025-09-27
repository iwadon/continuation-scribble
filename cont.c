#include "cont.h"
#include "m68k_ctx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
	if (m68k_ctx_save(&cont->ctx) != 0) {
		return 1;
	}
	char *sp = (char *)cont->ctx.a[7];
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

void cont_resume(const cont_t *cont)
{
	if (cont == NULL || cont->stack_image == NULL || cont->stack_size == 0) {
		if (cont_panic_fn != NULL) {
			cont_panic_fn("cont_resume: invalid argument");
		}
		abort();
	}
	void *p = malloc(cont->stack_size);
	if (p == NULL) {
		if (cont_panic_fn != NULL) {
			cont_panic_fn("cont_resume: out of memory");
		}
		abort();
	}
	memcpy(p, cont->stack_image, cont->stack_size);
	m68k_ctx_t ctx = cont->ctx;
	ctx.a[7] = (uint32_t)(char *)p;
	m68k_ctx_resume(&ctx);
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
