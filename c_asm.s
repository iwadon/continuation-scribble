	.xdef _m68k_ctx_save
	.xdef _m68k_ctx_resume

	.text
	.even

_m68k_ctx_save:
	movea.l	(4,sp),a0		* a0.l = &ctx
	move.l	(sp),(a0)		* ctx->ret = [SP] (戻りアドレス)
	movem.l	d0-d7/a0-a6,(4,a0)	* ctx->d[0..7] = D0-D7, ctx->a[0..6] = A0-A6
	move.l	a7,(64,a0)		* ctx->a[7] = A7
	move.w	sr,(68,a0)		* ctx->sr = SR
	moveq.l	#0,d0			* return 0 (保存側)
	rts

_m68k_ctx_resume:
	movea.l	(4,sp),a0		* a0.l = &ctx
	move.l	(a0),d1			* d1.l = ctx->ret (一時避難)
	move.l	(64,a0),a7		* A7 = ctx->a[7]
	movem.l	(12,a0),d2-d7/a0-a6	* ctx->d[2..7] = D2-D7, ctx->a[0..6] = A0-A6
	moveq.l	#1,d0			* return 1 (復帰側)
	movea.l	d1,a1			* a1.l = ctx->ret
	jmp	(a1)			* 保存地点の続きへ
