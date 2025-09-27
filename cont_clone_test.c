/* main.c : multi-shot の最小確認用 */
#include "cont.h"
#include <stdio.h>

/* 任意：panic ハンドラ（abort の前にメッセージを出したい場合） */
static void cont_panic(const char *msg) { printf("Panic: %s\n", msg); }

int main(void)
{
	/* 起動時 SP を cont_stack_base に格納（m68k, 68000想定） */
	asm volatile("move.l %%a7, %0" : "=r"(cont_stack_base));

	cont_panic_fn = cont_panic;

	cont_t base, c1, c2;
	cont_init(&base);
	cont_init(&c1);
	cont_init(&c2);
	printf("cont initialized.\n");

	if (cont_save(&base) == 0) {
		printf("cont saved once.\n");
		cont_clone(&c1, &base);
		cont_clone(&c2, &base);
		printf("resume c1\n");
		cont_resume(&c1);
		printf("We should not reach here.\n");
	}
	printf("resume c2\n");
	cont_resume(&c2);
	printf("We should not reach here too.\n");

	cont_term(&c2);
	cont_term(&c1);
	cont_term(&base);
	return 0;
}
