	.globl _fiber_trampoline

	.text
	.p2align 2

// fiber_trampoline (arm64)
// x21 = entry, x22 = arg
_fiber_trampoline:
	mov	x0, x22				// arg
	blr	x21				// entry(arg)
	bl	_fiber_mark_done_and_return	// noreturn
	b	.				// safety
