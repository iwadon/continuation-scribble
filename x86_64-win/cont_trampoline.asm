; cont_trampoline.asm - Windows x64 MASM implementation
; Trampoline for cont_resume to switch stack and call internal function
;
; Windows x64 calling convention:
;   - First argument (safe_sp): rcx
;   - Second argument (cont): rdx
;   - Third argument (fn): r8

.code

; void cont_resume_trampoline(char *safe_sp, const cont_t *cont, void (*fn)(const cont_t *))
;
; This function:
;   1. Moves RSP to safe_sp (below the stack area to be restored)
;   2. Calls fn(cont) which restores the stack and context
;   3. Never returns (fn calls cont_ctx_resume which is noreturn)
cont_resume_trampoline PROC
    ; Move stack pointer to safe location
    mov     rsp, rcx

    ; Allocate shadow space (32 bytes) required by Windows x64 ABI
    sub     rsp, 28h

    ; Setup argument for fn: cont -> rcx (first argument)
    mov     rcx, rdx

    ; Call fn (cont_resume_internal)
    call    r8

    ; Should never reach here since fn is noreturn
@@:
    jmp     @B
cont_resume_trampoline ENDP

END
