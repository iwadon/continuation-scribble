	.xdef _m68k_ctx_save
	.xdef _m68k_ctx_resume

	.text
	.even

_m68k_ctx_save:
	movea.l	(4,sp),a0		* a0.l = &ctx
	move.l	(sp),(a0)		* ctx->ret = [SP] (–ß‚èƒAƒhƒŒƒX)
	movem.l	d0-d7/a0-a6,(4,a0)	* ctx->d[0..7] = D0-D7, ctx->a[0..6] = A0-A6
	move.l	a7,(64,a0)		* ctx->a[7] = A7
	move.w	sr,(68,a0)		* ctx->sr = SR
	move.l	#0,d0			* return 0 (•Û‘¶‘¤)
	rts

_m68k_ctx_resume:
	rts
