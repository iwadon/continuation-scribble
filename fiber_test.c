#include "scheduler.h"
#include <stdio.h>

static void task(void *p)
{
	const char *name = (const char *)p;
	for (int i = 1; i <= 2; ++i) {
		printf("%s: %d\n", name, i);
		fiber_yield(); /* yield back to scheduler(main) */
	}
	printf("%s: done\n", name);
}

int main(void)
{
	scheduler_reset();

	fiber_t *f1, *f2;
	fiber_create(&f1, 16 * 1024, task, "F1");
	fiber_create(&f2, 16 * 1024, task, "F2");

	scheduler_add(f1);
	scheduler_add(f2);

	puts("-- start --");
	scheduler_run_all();
	puts("-- back to main --");

	fiber_destroy(f2);
	fiber_destroy(f1);
	return 0;
}
