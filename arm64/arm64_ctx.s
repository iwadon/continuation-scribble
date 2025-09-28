	.globl	_arm64_ctx_save
	.globl	_arm64_ctx_resume

	.text
	.p2align	2

_arm64_ctx_save:
	stp	x19, x20, [x0, #0x00]
	stp	x21, x22, [x0, #0x10]
	stp	x23, x24, [x0, #0x20]
	stp	x25, x26, [x0, #0x30]
	stp	x27, x28, [x0, #0x40]

	stp	x29, x30, [x0, #0x50]

	mov	x1, sp
	str	x1, [x0, #0x60]

	stp	q8, q9, [x0, #0x70]
	stp	q10, q11, [x0, #0x90]
	stp	q12, q13, [x0, #0xb0]
	stp	q14, q15, [x0, #0xd0]

	mov	w0, #0
	ret

_arm64_ctx_resume:
	ldp	x19, x20, [x0, #0x00]
	ldp	x21, x22, [x0, #0x10]
	ldp	x23, x24, [x0, #0x20]
	ldp	x25, x26, [x0, #0x30]
	ldp	x27, x28, [x0, #0x40]

	ldp	x29, x30, [x0, #0x50]

	ldr	x1, [x0, #0x60]
	mov	sp, x1

	ldp	q8, q9, [x0, #0x70]
	ldp	q10, q11, [x0, #0x90]
	ldp	q12, q13, [x0, #0xb0]
	ldp	q14, q15, [x0, #0xd0]

	mov	w0, #1
	ret
