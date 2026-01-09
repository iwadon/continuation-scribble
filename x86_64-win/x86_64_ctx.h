#if !defined(X86_64_CTX_H_INCLUDED)
#define X86_64_CTX_H_INCLUDED

#include <stdint.h>

/*
 * Windows x64 context structure
 *
 * Windows x64 ABI has more callee-saved registers than System V AMD64:
 *   - rbx, rbp, rdi, rsi, r12, r13, r14, r15
 *
 * Structure layout (offsets must match x86_64_ctx.asm):
 *   0x00: rbx
 *   0x08: rbp
 *   0x10: rdi
 *   0x18: rsi
 *   0x20: r12
 *   0x28: r13
 *   0x30: r14
 *   0x38: r15
 *   0x40: rsp
 *   0x48: rip
 */
typedef struct
{
	uint64_t rbx;
	uint64_t rbp;
	uint64_t rdi;
	uint64_t rsi;
	uint64_t r12;
	uint64_t r13;
	uint64_t r14;
	uint64_t r15;
	uint64_t rsp;
	uint64_t rip;
} x86_64_ctx_t;

#ifdef __cplusplus
extern "C" {
#endif

extern int x86_64_ctx_save(x86_64_ctx_t *c);

#ifdef _MSC_VER
__declspec(noreturn)
#else
__attribute__((noreturn))
#endif
extern void x86_64_ctx_resume(const x86_64_ctx_t *c);

#ifdef __cplusplus
}
#endif

#endif // X86_64_CTX_H_INCLUDED
