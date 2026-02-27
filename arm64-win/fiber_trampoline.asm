; fiber_trampoline.asm - Windows ARM64 implementation
; Entry point for newly created fibers
;
; Windows ARM64 calling convention:
;   - First argument: x0
;   - Callee-saved GPRs: x19-x28
;   - No shadow space required (unlike x86_64-win)

	IMPORT	fiber_mark_done_and_return

	AREA	|.text|, CODE, READONLY, ALIGN=4

; This is the first entry point for a new fiber.
; The context was prepared in fiber_create, with:
;   - x21 holding the entry function pointer
;   - x22 holding the argument
	EXPORT	fiber_trampoline
fiber_trampoline PROC
	; Per Windows ARM64 ABI, the first argument is passed in x0.
	mov	x0, x22

	; Call the entry function. The address is in x21.
	blr	x21

	; If the fiber's entry function returns, it should be terminated.
	bl	fiber_mark_done_and_return	; This function never returns.

	; Safeguard infinite loop in case noreturn is violated.
0
	b	%B0
	ENDP

	END
