#include "c.h"

int main(void)
{
	m68k_ctx_t ctx;
	m68k_ctx_save(&ctx);
	m68k_ctx_resume(&ctx);
	return 0;
}
