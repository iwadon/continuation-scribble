#ifndef SCHEDULER_H_INCLUDED
#define SCHEDULER_H_INCLUDED

#include "fiber.h"

/* Very small round-robin scheduler (cooperative). */
void scheduler_reset(void);
int scheduler_add(fiber_t *fb); /* returns index or <0 on error */
void scheduler_run_all(void);	/* returns when all fibers are DONE */

#endif // SCHEDULER_H_INCLUDED
