#include "c.h"
#include <stdio.h>

int main(void)
{
	m68k_ctx_t ctx;
	m68k_ctx_save(&ctx);
	printf("RET %08lx\n", ctx.ret);
	printf("D   %08lx %08lx %08lx %08lx %08lx %08lx %08lx %08lx\n", ctx.d[0], ctx.d[1], ctx.d[2], ctx.d[3], ctx.d[4], ctx.d[5], ctx.d[6], ctx.d[7]);
	printf("A   %08lx %08lx %08lx %08lx %08lx %08lx %08lx %08lx\n", ctx.a[0], ctx.a[1], ctx.a[2], ctx.a[3], ctx.a[4], ctx.a[5], ctx.a[6], ctx.a[7]);
	printf("SR  %04x\n", ctx.sr);
	m68k_ctx_resume(&ctx);
	return 0;
}
