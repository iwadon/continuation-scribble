#include "x86_64_ctx.h"
#include <stdio.h>
#include <stdlib.h>

x86_64_ctx_t main_ctx, child_ctx;
char child_stack[4096];
volatile int flag = 0;

void child_main(void) {
    printf("Entered child context.\n");
    flag = 1;
    printf("Switching back to main context.\n");
    x86_64_ctx_swap(&child_ctx, &main_ctx);
    printf("Should not be here.\n");
}

int main(void) {
    // Setup child context
    uint64_t *sp = (uint64_t *)((child_stack + sizeof(child_stack)));

    // The 'ret' from x86_64_ctx_swap will jump to child_main.
    *--sp = (uint64_t)child_main;

    // Dummy space for the 6 callee-saved registers that x86_64_ctx_swap will 'pop'.
    *--sp = 0; // r15
    *--sp = 0; // r14
    *--sp = 0; // r13
    *--sp = 0; // r12
    *--sp = 0; // rbp
    *--sp = 0; // rbx

    child_ctx.rsp = (uint64_t)sp;

    printf("Switching to child context...\n");
    
    // Swap to child. It will run and eventually swap back to us.
    x86_64_ctx_swap(&main_ctx, &child_ctx);

    printf("Resumed in main context.\n");

    if (flag == 1) {
        printf("OK: Flag was set by child.\n");
        return 0;
    } else {
        printf("ERROR: Flag was not set.\n");
        return 1;
    }
}