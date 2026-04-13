	.xdef _m68k_ctx_save
	.xdef _m68k_ctx_resume

	.text
	.even

_m68k_ctx_save:
	movea.l	(4,sp),a0		* a0.l = &ctx
	move.l	(sp),(a0)		* ctx->ret = [SP] (return address)
	movem.l	d0-d7/a0-a6,(4,a0)	* ctx->d[0..7] = D0-D7, ctx->a[0..6] = A0-A6
	lea	(4,sp),a1		* a1 = SP after rts would pop retaddr (caller's SP)
	move.l	a1,(64,a0)		* ctx->a[7] = caller's SP (post-rts)
	move.w	sr,(68,a0)		* ctx->sr = SR
	moveq.l	#0,d0			* return 0 (save path)
	rts

_m68k_ctx_resume:
	movea.l	(4,sp),a0		* a0.l = &ctx
	move.l	(64,a0),a7		* A7 = ctx->a[7] (caller's original SP)
	move.l	(a0),-(sp)		* push ctx->ret as return addr for rts
	movem.l	(8,a0),d1-d7/a0-a6	* restore D1-D7, A0-A6
	moveq.l	#1,d0			* return 1 (resume path)
	rts				* pop ret and jump; SP ends at caller's SP
