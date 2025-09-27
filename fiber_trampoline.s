	.xdef	_fiber_trampoline
	.xref	_fiber_mark_done_and_return

	.text
	.even

_fiber_trampoline:
	move.l	d2,-(sp)			* push arg
	movea.l	a2,a0				* func -> a0
	jsr	(a0)				* call func(arg)
	addq.l	#4,sp				* pop arg
	jsr	_fiber_mark_done_and_return	* exit fiber
	bra.s	_fiber_trampoline		* should not return
