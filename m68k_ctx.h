#if !defined(M68K_CTX_H_INCLUDED)
#define M68K_CTX_H_INCLUDED

#include <stdint.h>

#if defined(__human68k__)

typedef struct
{
	uint32_t ret;  // Return address
	uint32_t d[8]; // d0-d7
	uint32_t a[8]; // a0-a7
	uint16_t sr;   // Status register
} m68k_ctx_t;

extern int m68k_ctx_save(m68k_ctx_t *c);
extern void m68k_ctx_resume(const m68k_ctx_t *c);

#endif // __human68k__

#endif // M68K_CTX_H_INCLUDED
