; arm64_ctx.asm - Windows ARM64 implementation
; Context save/restore for Windows ARM64 calling convention
;
; Windows ARM64 calling convention:
;   - First argument: x0
;   - Callee-saved GPRs: x19-x28, x29 (fp), x30 (lr)
;   - Callee-saved SIMD: d8-d15 (lower 64-bit only; upper 64 bits are volatile)
;   - x18 is reserved (TEB pointer) and must not be touched
;   - 16-byte stack alignment required

	AREA	|.text|, CODE, READONLY, ALIGN=4

; int arm64_ctx_save(arm64_ctx_t *c)
; x0: pointer to context structure
; Returns 0 on save, 1 on resume
	EXPORT	arm64_ctx_save
arm64_ctx_save PROC
	; Save callee-saved general purpose registers (X19-X28)
	stp	x19, x20, [x0, #0x00]
	stp	x21, x22, [x0, #0x10]
	stp	x23, x24, [x0, #0x20]
	stp	x25, x26, [x0, #0x30]
	stp	x27, x28, [x0, #0x40]

	; Save frame pointer and link register
	stp	x29, x30, [x0, #0x50]

	; Save stack pointer
	mov	x1, sp
	str	x1, [x0, #0x60]

	; Save callee-saved SIMD registers (D8-D15, 64-bit only)
	stp	d8, d9, [x0, #0x68]
	stp	d10, d11, [x0, #0x78]
	stp	d12, d13, [x0, #0x88]
	stp	d14, d15, [x0, #0x98]

	; Return 0, indicating we saved the context
	mov	w0, #0
	ret
	ENDP

; void arm64_ctx_resume(const arm64_ctx_t *c)
; x0: pointer to context structure
; This function never returns (noreturn)
	EXPORT	arm64_ctx_resume
arm64_ctx_resume PROC
	; Restore callee-saved general purpose registers (X19-X28)
	ldp	x19, x20, [x0, #0x00]
	ldp	x21, x22, [x0, #0x10]
	ldp	x23, x24, [x0, #0x20]
	ldp	x25, x26, [x0, #0x30]
	ldp	x27, x28, [x0, #0x40]

	; Restore frame pointer and link register
	ldp	x29, x30, [x0, #0x50]

	; Restore stack pointer
	ldr	x1, [x0, #0x60]
	mov	sp, x1

	; Restore callee-saved SIMD registers (D8-D15, 64-bit only)
	ldp	d8, d9, [x0, #0x68]
	ldp	d10, d11, [x0, #0x78]
	ldp	d12, d13, [x0, #0x88]
	ldp	d14, d15, [x0, #0x98]

	; Return value = 1 for resumed context
	mov	w0, #1
	ret
	ENDP

	END
