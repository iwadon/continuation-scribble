#if !defined(ARM64_CTX_H_INCLUDED)
#define ARM64_CTX_H_INCLUDED

#include <stdint.h>

/*
 * Windows ARM64 context structure
 *
 * Windows ARM64 ABI callee-saved registers:
 *   - X19-X28 (general purpose)
 *   - X29 (frame pointer, required for ETW)
 *   - X30 (link register)
 *   - D8-D15 (lower 64 bits of V8-V15; upper 64 bits are volatile)
 *   - X18 is reserved (TEB pointer) and must not be modified
 *
 * Structure layout (offsets must match arm64_ctx.asm):
 *   0x00: X19-X28 (10 * 8 = 80 bytes)
 *   0x50: FP (X29)
 *   0x58: LR (X30)
 *   0x60: SP
 *   0x68: D8-D15 (8 * 8 = 64 bytes)
 */
typedef struct
{
	uint64_t x[10];  /* 0x00  Callee-saved GPRs: X19-X28 */
	uint64_t fp;     /* 0x50  X29 (Frame Pointer) */
	uint64_t lr;     /* 0x58  X30 (Link Register) */
	uint64_t sp;     /* 0x60  Stack Pointer */
	uint64_t d[8];   /* 0x68  Callee-saved SIMD: D8-D15 (64-bit only) */
} arm64_ctx_t;

#ifdef __cplusplus
extern "C" {
#endif

extern int arm64_ctx_save(arm64_ctx_t *c);

#ifdef _MSC_VER
__declspec(noreturn)
#else
__attribute__((noreturn))
#endif
extern void arm64_ctx_resume(const arm64_ctx_t *c);

#ifdef __cplusplus
}
#endif

#endif // ARM64_CTX_H_INCLUDED
