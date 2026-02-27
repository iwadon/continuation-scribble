; cont_trampoline.asm - Windows ARM64 implementation
; Trampoline for cont_resume to switch stack and call internal function
;
; Windows ARM64 calling convention:
;   - First argument (safe_sp): x0
;   - Second argument (cont): x1
;   - Third argument (fn): x2
;   - No shadow space required (unlike x86_64-win)

	AREA	|.text|, CODE, READONLY, ALIGN=4

; void cont_resume_trampoline(char *safe_sp, const cont_t *cont, void (*fn)(const cont_t *))
;
; This function:
;   1. Moves SP to safe_sp (below the stack area to be restored)
;   2. Calls fn(cont) which restores the stack and context
;   3. Never returns (fn calls cont_ctx_resume which is noreturn)
	EXPORT	cont_resume_trampoline
cont_resume_trampoline PROC
	; Move stack pointer to safe location
	mov	sp, x0

	; Setup argument for fn: cont -> x0 (first argument)
	mov	x0, x1

	; Call fn (cont_resume_internal)
	blr	x2

	; Should never reach here since fn is noreturn
0
	b	%B0
	ENDP

	END
