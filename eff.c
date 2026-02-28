#include "eff.h"

/* Global state */
eff_handler_t *eff_handler_stack = NULL;
int eff_tag = 0;
intptr_t eff_arg = 0;
eff_cont_t eff_k;
intptr_t eff_resume_val = 0;

intptr_t eff_perform(int tag, intptr_t arg)
{
	eff_handler_t *h = eff_handler_stack;

	/* Set up globals before jumping to handler */
	eff_tag = tag;
	eff_arg = arg;
	cont_init(&eff_k.cont);

	/*
	 * Save the perform site as a continuation.
	 * cont_save returns 0 on initial save, 1 when resumed.
	 */
	if (cont_save(&eff_k.cont) == 0) {
		/* Initial save: jump to handler's escape continuation */
		cont_resume(&h->escape);
		/* unreachable */
	}

	/* Resumed via eff_resume — return the value */
	return eff_resume_val;
}

void eff_resume(eff_cont_t *k, intptr_t val)
{
	eff_resume_val = val;
	cont_resume(&k->cont);
	/* unreachable */
}

void eff_reperform(eff_handler_t *h, int tag, intptr_t arg, eff_cont_t *k)
{
	/* Save the original continuation from the computation */
	eff_cont_t saved_k;
	cont_init(&saved_k.cont);
	cont_clone(&saved_k.cont, &k->cont);

	/* Pop self from handler stack so perform reaches the outer handler */
	eff_handler_stack = h->prev;

	/* Perform the effect to the outer handler */
	intptr_t result = eff_perform(tag, arg);

	/* Outer handler resumed us — restore self on the stack */
	eff_handler_stack = h;

	/* Resume the original computation with the value from the outer handler */
	eff_resume_val = result;
	cont_resume(&saved_k.cont);
	/* unreachable */
}

void eff_handle_end(eff_handler_t *h)
{
	/* Pop handler from stack */
	eff_handler_stack = h->prev;
	/* Restore original stack base */
	cont_stack_base = h->saved_stack_base;
	/* Clean up the escape continuation */
	cont_term(&h->escape);
}
