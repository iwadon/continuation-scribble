#if !defined(ARM64_CTX_H_INCLUDED)
#define ARM64_CTX_H_INCLUDED

#include <stdint.h>

typedef struct
{
	uint64_t x[10];	   // 0x00 Callee-saved General Purpose Registers (X19 - X28) (10 * 8 bytes = 80 bytes)
	uint64_t fp;	   // 0x50 X29 (Frame Pointer)
	uint64_t lr;	   // 0x58 X30 (Link Register, Return Address/Resume Point)
	uint64_t sp;	   // 0x60 SP (Stack Pointer)
	uint64_t reserved; // 0x68 Reserved for alignment
	__uint128_t q[8]; // 0x70 Callee-saved Floating Point/Vector Registers (Q8 - Q15) (8 * 16 bytes = 128 bytes)
} arm64_ctx_t;

extern int arm64_ctx_save(arm64_ctx_t *c);
__attribute__((noreturn)) extern void arm64_ctx_resume(const arm64_ctx_t *c);

#endif // ARM64_CTX_H_INCLUDED
