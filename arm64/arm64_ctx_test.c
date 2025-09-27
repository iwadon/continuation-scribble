#include "arm64_ctx.h"
#include <stdio.h>

int main(void)
{
	arm64_ctx_t ctx;
	if (arm64_ctx_save(&ctx) == 0) {
		printf("Context saved outside.\n");
		arm64_ctx_t ctx2;
		if (arm64_ctx_save(&ctx2) == 0) {
			printf("Context saved inside.\n");
			arm64_ctx_resume(&ctx2);
			printf("You shouldn't come here.\n");
		}
		printf("Context restored inside.\n");
		arm64_ctx_resume(&ctx);
		printf("You shouldn't come here too.\n");
	}
	printf("Context restored outside.\n");
	return 0;
}
