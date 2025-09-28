#if !defined(X86_64_CTX_H_INCLUDED)
#define X86_64_CTX_H_INCLUDED

#include <stdint.h>

typedef struct
{
	uint64_t rbx;
	uint64_t rbp;
	uint64_t r12;
	uint64_t r13;
	uint64_t r14;
	uint64_t r15;
	uint64_t rsp;
	uint64_t rip;
} x86_64_ctx_t;

extern int x86_64_ctx_save(x86_64_ctx_t *c);
__attribute__((noreturn)) extern void x86_64_ctx_resume(const x86_64_ctx_t *c);

#endif // X86_64_CTX_H_INCLUDED