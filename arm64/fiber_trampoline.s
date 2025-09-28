	.globl _fiber_trampoline

	.text
	.p2align 2

// fiber_trampoline (arm64)
// x2 = entry, x3 = arg
_fiber_trampoline:
	stp     x3, x30, [sp, #-16]!   		// arg をスタック経由で渡す（ABI準拠）
	mov     x0, x3
	blr     x2                     		// entry(arg)
	add     sp, sp, #16
	bl      _fiber_mark_done_and_return	// noreturn
	b       .                      		// safety
