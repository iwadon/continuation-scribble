#include "x86_64_ctx.h"
#include <stdio.h>

int main(void)
{
	x86_64_ctx_t ctx;
	if (x86_64_ctx_save(&ctx) == 0) {
		printf("Context saved outside.\n");
		x86_64_ctx_t ctx2;
		if (x86_64_ctx_save(&ctx2) == 0) {
			printf("Context saved inside.\n");
			x86_64_ctx_resume(&ctx2);
			printf("You shouldn't come here.\n");
		}
		printf("Context restored inside.\n");
		x86_64_ctx_resume(&ctx);
		printf("You shouldn't come here too.\n");
	}
	printf("Context restored outside.\n");
	return 0;
}
