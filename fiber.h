#if !defined(FIBER_H_INCLUDED)
#define FIBER_H_INCLUDED

#include <stddef.h>

#if defined(__GNUC__)
#define FIBER_NORETURN __attribute__((noreturn))
#else
#define FIBER_NORETURN
#endif

/* Opaque handle */
typedef struct fiber fiber_t;

typedef enum {
	FIB_READY = 0,
	FIB_RUNNING,
	FIB_DONE
} fib_state_t;

/* One-time: associate a fiber handle with the main thread (no heap stack). */
int fiber_main_init(fiber_t **out_main);

/* Create/destroy a fiber with its own stack. */
int fiber_create(fiber_t **out, size_t stack_size, void (*entry)(void *), void *arg);
void fiber_destroy(fiber_t *fb);

/* State/query */
fib_state_t fiber_state(const fiber_t *fb);
int fiber_is_done(const fiber_t *fb);

/* Cooperative switching */
void fiber_yield(void);			  /* yield to main (scheduler) */
void fiber_yield_to(fiber_t *to); /* yield to another fiber */

/* Mark current fiber done and return to main (never schedules this fiber again) */
void FIBER_NORETURN fiber_mark_done_and_return(void);

/* Current/main helpers */
fiber_t *fiber_self(void);
fiber_t *fiber_main(void);

#endif // FIBER_H_INCLUDED
