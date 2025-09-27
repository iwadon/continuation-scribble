#include "m68k_ctx.h"
#include <stdio.h>

void *stack_base;

int main(void)
{
	m68k_ctx_t ctx;
	if (m68k_ctx_save(&ctx) == 0) {
		printf("Context saved outside.\n");
		m68k_ctx_t ctx2;
		if (m68k_ctx_save(&ctx2) == 0) {
			printf("Context saved inside.\n");
			m68k_ctx_resume(&ctx2);
			printf("You shouldn't come here.\n");
		}
		printf("Context restored inside.\n");
		m68k_ctx_resume(&ctx);
		printf("You shouldn't come here too.\n");
	}
	printf("Context restored outside.\n");
	return 0;
}
