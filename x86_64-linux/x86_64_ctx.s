	.globl	x86_64_ctx_save
	.globl	x86_64_ctx_resume

	.text
	.p2align	2

# int x86_64_ctx_save(x86_64_ctx_t *c)
# rdi: c
x86_64_ctx_save:
	# Save callee-saved registers
	movq	%rbx, 0x00(%rdi)
	movq	%rbp, 0x08(%rdi)
	movq	%r12, 0x10(%rdi)
	movq	%r13, 0x18(%rdi)
	movq	%r14, 0x20(%rdi)
	movq	%r15, 0x28(%rdi)

	# Save stack pointer. We want the SP of our caller.
	lea	8(%rsp), %rax
	movq	%rax, 0x30(%rdi)

	# Save our return address (RIP)
	movq	(%rsp), %rax
	movq	%rax, 0x38(%rdi)

	# Return 0, indicating we saved the context and are returning normally.
	xorl	%eax, %eax
	ret

# void x86_64_ctx_resume(const x86_64_ctx_t *c) __attribute__((noreturn))
# rdi: c
x86_64_ctx_resume:
	# Restore stack pointer from context
	movq	0x30(%rdi), %rsp

	# Restore callee-saved registers from context
	movq	0x00(%rdi), %rbx
	movq	0x08(%rdi), %rbp
	movq	0x10(%rdi), %r12
	movq	0x18(%rdi), %r13
	movq	0x20(%rdi), %r14
	movq	0x28(%rdi), %r15

	# Set the return value to 1 for the restored context.
	# The code that called save will see 1 as the return value.
	movl	$1, %eax

	# Jump to the restored instruction pointer.
	jmp	*0x38(%rdi)

# Mark stack as non-executable (required by modern Linux linkers)
	.section .note.GNU-stack,"",@progbits
