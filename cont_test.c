#include "cont.h"
#include <stdio.h>

void cont_panic(const char *msg)
{
	printf("Panic: %s\n", msg);
}

int main(void)
{
	asm volatile("move.l %%a7, %0" : "=r"(cont_stack_base));
	cont_panic_fn = cont_panic;
	cont_t cont;
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
	return 0;
}
