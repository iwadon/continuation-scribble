#if !defined(C_H_INCLUDED)
#define C_H_INCLUDED

#include <stdint.h>

typedef struct
{
	uint32_t ret;  // Return address
	uint32_t d[8]; // d0-d7
	uint32_t a[8]; // a0-a7
	uint16_t sr;   // Status register
} m68k_ctx_t;

int m68k_ctx_save(m68k_ctx_t *c);
void m68k_ctx_resume(const m68k_ctx_t *c);

#endif // C_H_INCLUDED
