; x86_64_ctx.asm - Windows x64 MASM implementation
; Context save/restore for Windows x64 calling convention
;
; Windows x64 calling convention:
;   - First argument: rcx
;   - Callee-saved registers: rbx, rbp, rdi, rsi, r12-r15
;   - 16-byte stack alignment required before CALL

.code

; int x86_64_ctx_save(x86_64_ctx_t *c)
; rcx: pointer to context structure
; Returns 0 on save, 1 on resume
x86_64_ctx_save PROC
    ; Save callee-saved registers
    mov     QWORD PTR [rcx + 00h], rbx
    mov     QWORD PTR [rcx + 08h], rbp
    mov     QWORD PTR [rcx + 10h], rdi
    mov     QWORD PTR [rcx + 18h], rsi
    mov     QWORD PTR [rcx + 20h], r12
    mov     QWORD PTR [rcx + 28h], r13
    mov     QWORD PTR [rcx + 30h], r14
    mov     QWORD PTR [rcx + 38h], r15

    ; Save stack pointer. We want the SP of our caller.
    ; Current RSP points to return address, so caller's RSP = RSP + 8
    lea     rax, [rsp + 8]
    mov     QWORD PTR [rcx + 40h], rax

    ; Save return address (RIP)
    mov     rax, QWORD PTR [rsp]
    mov     QWORD PTR [rcx + 48h], rax

    ; Return 0, indicating we saved the context
    xor     eax, eax
    ret
x86_64_ctx_save ENDP

; void x86_64_ctx_resume(const x86_64_ctx_t *c)
; rcx: pointer to context structure
; This function never returns (noreturn)
x86_64_ctx_resume PROC
    ; Restore stack pointer first
    mov     rsp, QWORD PTR [rcx + 40h]

    ; Restore callee-saved registers
    mov     rbx, QWORD PTR [rcx + 00h]
    mov     rbp, QWORD PTR [rcx + 08h]
    mov     rdi, QWORD PTR [rcx + 10h]
    mov     rsi, QWORD PTR [rcx + 18h]
    mov     r12, QWORD PTR [rcx + 20h]
    mov     r13, QWORD PTR [rcx + 28h]
    mov     r14, QWORD PTR [rcx + 30h]
    mov     r15, QWORD PTR [rcx + 38h]

    ; Return value = 1 for resumed context
    mov     eax, 1

    ; Jump to saved instruction pointer
    jmp     QWORD PTR [rcx + 48h]
x86_64_ctx_resume ENDP

END
