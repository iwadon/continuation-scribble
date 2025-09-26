#include "c.h"
#include <stdio.h>

void *stack_base;

int main(void)
{
#if defined(__human68k__)
	m68k_ctx_t ctx;
	if (m68k_ctx_save(&ctx) == 0) {
		printf("Context saved.\n");
		m68k_ctx_resume(&ctx);
		printf("You shouldn't come here.\n");
	}
	printf("Context restored.\n");
#endif
	return 0;
}
