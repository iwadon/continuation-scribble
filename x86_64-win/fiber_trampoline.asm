; fiber_trampoline.asm - Windows x64 MASM implementation
; Entry point for newly created fibers
;
; Windows x64 calling convention:
;   - First argument: rcx
;   - Callee-saved registers: rbx, rbp, rdi, rsi, r12-r15

EXTERN fiber_mark_done_and_return:PROC

.code

; This is the first entry point for a new fiber.
; The context was prepared in fiber_create, with:
;   - rbx holding the entry function pointer
;   - r12 holding the argument
fiber_trampoline PROC
    ; Per Windows x64 ABI, the first argument is passed in rcx.
    mov     rcx, r12

    ; Allocate shadow space (32 bytes) required by Windows x64 ABI
    sub     rsp, 28h

    ; Call the entry function. The address is in rbx.
    call    rbx

    ; If the fiber's entry function returns, it should be terminated.
    call    fiber_mark_done_and_return  ; This function never returns.

    ; Safeguard infinite loop in case noreturn is violated.
@@:
    jmp     @B
fiber_trampoline ENDP

END
