#include "m68k_ctx.h"
#include <stdio.h>

void *stack_base;

static inline void *read_sp(void)
{
	void *sp;
	__asm__ volatile("move.l %%sp,%0" : "=r"(sp));
	return sp;
}

static int failures;

/*
 * SP保存バグの回帰テスト。
 * save/resumeを経由してもcaller側のSPと、スタック上のローカル変数値が
 * 保たれていることを検証する。以前のバグでは resume 後にSPが4バイトずれ、
 * ローカル変数領域へのアクセスがズレる症状があった。
 */
static void test_sp_roundtrip(void)
{
	volatile uint32_t canary_before = 0xDEADBEEFu;
	volatile uint32_t canary_after = 0x12345678u;
	void *sp_before = read_sp();

	m68k_ctx_t ctx;
	volatile int resumed = 0;
	if (m68k_ctx_save(&ctx) == 0) {
		if (!resumed) {
			resumed = 1;
			m68k_ctx_resume(&ctx);
		}
	}

	void *sp_after = read_sp();
	if (sp_before != sp_after) {
		printf("FAIL: SP mismatch: before=%p after=%p (diff=%ld)\n",
			sp_before, sp_after,
			(long)((char *)sp_after - (char *)sp_before));
		failures++;
	} else {
		printf("OK: SP preserved across save/resume (%p)\n", sp_before);
	}

	if (canary_before != 0xDEADBEEFu || canary_after != 0x12345678u) {
		printf("FAIL: stack canary corrupted: %08lx %08lx\n",
			(unsigned long)canary_before, (unsigned long)canary_after);
		failures++;
	} else {
		printf("OK: stack canaries intact\n");
	}
}

/*
 * callee内でsave/resumeしてもcallerのスタックフレームが壊れないことを確認。
 * SP +4 バグがあるとcalleeリターン後にcallerのローカル変数が化ける可能性がある。
 */
static void callee_save_resume(void)
{
	m68k_ctx_t ctx;
	volatile int resumed = 0;
	if (m68k_ctx_save(&ctx) == 0) {
		if (!resumed) {
			resumed = 1;
			m68k_ctx_resume(&ctx);
		}
	}
}

static void test_caller_frame_intact(void)
{
	volatile uint32_t a = 0xAAAAAAAAu;
	volatile uint32_t b = 0xBBBBBBBBu;
	volatile uint32_t c = 0xCCCCCCCCu;
	void *sp_before = read_sp();

	callee_save_resume();

	void *sp_after = read_sp();
	if (sp_before != sp_after) {
		printf("FAIL: caller SP moved across callee save/resume: %p -> %p\n",
			sp_before, sp_after);
		failures++;
	} else {
		printf("OK: caller SP unchanged across callee save/resume\n");
	}

	if (a != 0xAAAAAAAAu || b != 0xBBBBBBBBu || c != 0xCCCCCCCCu) {
		printf("FAIL: caller locals clobbered: %08lx %08lx %08lx\n",
			(unsigned long)a, (unsigned long)b, (unsigned long)c);
		failures++;
	} else {
		printf("OK: caller locals intact\n");
	}
}

int main(void)
{
	m68k_ctx_t ctx;
	if (m68k_ctx_save(&ctx) == 0) {
		printf("Context saved outside.\n");
		m68k_ctx_t ctx2;
		if (m68k_ctx_save(&ctx2) == 0) {
			printf("Context saved inside.\n");
			m68k_ctx_resume(&ctx2);
			printf("You shouldn't come here.\n");
		}
		printf("Context restored inside.\n");
		m68k_ctx_resume(&ctx);
		printf("You shouldn't come here too.\n");
	}
	printf("Context restored outside.\n");

	printf("\n-- SP regression tests --\n");
	test_sp_roundtrip();
	test_caller_frame_intact();

	if (failures) {
		printf("FAILED: %d check(s)\n", failures);
		return 1;
	}
	printf("ALL PASS\n");
	return 0;
}
