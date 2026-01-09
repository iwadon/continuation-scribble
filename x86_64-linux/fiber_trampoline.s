	.globl fiber_trampoline

	.text
	.p2align 2

fiber_trampoline:
	# This is the first entry point for a new fiber.
	# The context was prepared in fiber_create, with:
	# - %rbx holding the entry function pointer
	# - %r12 holding the argument

	# Per System V AMD64 ABI, the first argument is passed in %rdi.
	movq %r12, %rdi

	# Call the entry function. The address is in %rbx.
	call *%rbx

	# If the fiber's entry function returns, it should be terminated.
	call fiber_mark_done_and_return # This function never returns.

	# Safeguard infinite loop in case noreturn is violated.
	jmp .

# Mark stack as non-executable (required by modern Linux linkers)
	.section .note.GNU-stack,"",@progbits
