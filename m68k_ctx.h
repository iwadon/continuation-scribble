#if !defined(M68K_CTX_H_INCLUDED)
#define M68K_CTX_H_INCLUDED

#include <stdint.h>

typedef struct
{
	uint32_t ret;  // 0x00 1.l Return address
	uint32_t d[8]; // 0x04 8.l d0-d7
	uint32_t a[8]; // 0x24 8.l a0-a7
	uint16_t sr;   // 0x44 1.w Status register
} m68k_ctx_t;

extern int m68k_ctx_save(m68k_ctx_t *c);
__attribute__((noreturn)) extern void m68k_ctx_resume(const m68k_ctx_t *c);

#endif // M68K_CTX_H_INCLUDED
