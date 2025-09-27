#include "scheduler.h"
#include <assert.h>
#include <string.h>

#define MAX_FIBERS 32
static fiber_t *rq[MAX_FIBERS];
static int rq_n = 0;
static fiber_t *g_main_fb = 0;

void scheduler_reset(void)
{
	rq_n = 0;
	fiber_main_init(&g_main_fb);
}

int scheduler_add(fiber_t *fb)
{
	if (!fb || rq_n >= MAX_FIBERS) {
		return -1;
	}
	rq[rq_n++] = fb;
	return rq_n - 1;
}

void scheduler_run_all(void)
{
	if (!g_main_fb) {
		fiber_main_init(&g_main_fb);
	}
	for (;;) {
		int alive = 0;
		for (int i = 0; i < rq_n; ++i) {
			fiber_t *fb = rq[i];
			if (!fb || fiber_is_done(fb)) {
				continue;
			}
			alive = 1;
			fiber_yield_to(fb); /* switch to fiber; it yields back */
		}
		if (!alive) {
			break; /* all done */
		}
	}
}
