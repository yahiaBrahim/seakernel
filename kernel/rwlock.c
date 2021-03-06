/* rwlock.c - copyright Daniel Bittman 2013
 * implements rwlock procedures
 *
 * each rwlock may have any number of readers, but only one writer. Also
 * a writer may only clench a lock if there are zero readers, and if
 * a writer has the lock, no readers may lock it. If a lock cannot be
 * acquired, it will go into sleep until the lock can be acquired
 */
#include <sea/kernel.h>
#include <stdatomic.h>
#include <sea/rwlock.h>
#include <sea/tm/process.h>
#include <sea/mm/kmalloc.h>
#include <sea/kobj.h>
#define RWLOCK_DEBUG 0
void __rwlock_acquire(struct rwlock *lock, enum rwlock_locktype type, char *file, int line)
{
	if(kernel_state_flags & KSF_DEBUGGING)
		return;
	if(current_thread && current_thread->interrupt_level)
		panic(PANIC_NOSYNC, "cannot lock an rwlock within interrupt context");
	assert(lock->magic == RWLOCK_MAGIC);
	assertmsg(!current_thread || (__current_cpu->preempt_disable == 0),
			"tried to rwlock with preempt disabled");
	if(kernel_state_flags & KSF_SHUTDOWN) return;
#if RWLOCK_DEBUG
	int timeout = 500;
#endif
	if(current_thread) {
		current_thread->held_locks++;
	}
	while(atomic_flag_test_and_set_explicit(&lock->writer, memory_order_acq_rel)) {
#if RWLOCK_DEBUG
		if(!--timeout)
			panic(0,
					"timeout1: %s %d (held by %s:%d)",
					file, line, lock->holderfile ? lock->holderfile : "?", lock->holderline);
#endif
		tm_schedule();
	}

	if(type == RWL_READER) {
		atomic_fetch_add(&lock->readers, 1);
		atomic_flag_clear_explicit(&lock->writer, memory_order_release);
	} else {
#if RWLOCK_DEBUG
		timeout = 50000000;
#endif
		while(atomic_load(&lock->readers) != 0) {
#if RWLOCK_DEBUG
			if(!--timeout) panic(0, "timeout2: %s %d", file, line);
#endif
			tm_schedule();
		}
	}
	lock->holderline = line;
	lock->holderfile = file;
}

void __rwlock_deescalate(struct rwlock *lock, char *file, int line)
{
	assert(lock->magic == RWLOCK_MAGIC);
	if(kernel_state_flags & KSF_DEBUGGING)
		return;
	if(kernel_state_flags & KSF_SHUTDOWN) return;
	
	atomic_fetch_add_explicit(&lock->readers, 1, memory_order_acquire);
	atomic_flag_clear_explicit(&lock->writer, memory_order_release);
}

void rwlock_release(struct rwlock *lock, enum rwlock_locktype type)
{
	assert(lock->magic == RWLOCK_MAGIC);
	if(kernel_state_flags & KSF_DEBUGGING)
		return;
	if(kernel_state_flags & KSF_SHUTDOWN) return;
	lock->holderline=0;
	lock->holderfile=0;
	if(type == RWL_READER) {
		assert(lock->readers >= 1);
		atomic_fetch_sub_explicit(&lock->readers, 1, memory_order_release);
	} else {
		assert(lock->readers == 0);
		atomic_flag_clear_explicit(&lock->writer, memory_order_release);
	}
	if(current_thread)
		current_thread->held_locks--;
}

struct rwlock *rwlock_create(struct rwlock *lock)
{
	KOBJ_CREATE(lock, 0, RWL_ALLOC);
	lock->magic = RWLOCK_MAGIC;
	return lock;
}

void rwlock_destroy(struct rwlock *lock)
{
	assert(lock->magic == RWLOCK_MAGIC);
	if(kernel_state_flags & KSF_SHUTDOWN) return;
	assert(lock->readers == 0);
	assert(!atomic_flag_test_and_set(&lock->writer));
	lock->magic=0;
	KOBJ_DESTROY(lock, RWL_ALLOC);
}

