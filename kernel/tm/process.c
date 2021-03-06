#include <sea/tm/process.h>
#include <sea/kernel.h>
#include <sea/tm/process.h>
#include <sea/lib/hash.h>
#include <sea/errno.h>
#include <sea/lib/linkedlist.h>

struct hash *process_table;
struct linkedlist *process_list;
size_t running_processes = 0;

struct mutex process_refs_lock;

struct process *tm_process_get(pid_t pid)
{
	struct process *proc;
	mutex_acquire(&process_refs_lock);
	if((proc = hash_lookup(process_table, &pid, sizeof(pid))) == NULL) {
		mutex_release(&process_refs_lock);
		return 0;
	}
	atomic_fetch_add(&proc->refs, 1);
	mutex_release(&process_refs_lock);
	return proc;
}

void tm_process_inc_reference(struct process *proc)
{
	assert(proc->magic == PROCESS_MAGIC);
	if(!(proc->refs >= 1))
		panic(PANIC_NOSYNC, "process refcount error (inc): %d refs = %d\n", proc->pid, proc->refs);
	atomic_fetch_add(&proc->refs, 1);
}

void tm_process_put(struct process *proc)
{
	assert(proc->magic == PROCESS_MAGIC);
	mutex_acquire(&process_refs_lock);
	if(!(proc->refs >= 1))
		panic(PANIC_NOSYNC, "process refcount error (put): %d (%s) refs = %d\n", proc->pid, proc->command, proc->refs);
	if(atomic_fetch_sub(&proc->refs, 1) == 1) {
		hash_delete(process_table, &proc->pid, sizeof(proc->pid));
		mutex_release(&process_refs_lock);
		/* do this here...since we must wait for every thread to give up
		 * their refs. This happens in schedule, after it gets scheduled away */
		mm_context_destroy(&proc->vmm_context);
		kfree(proc);
	} else {
		mutex_release(&process_refs_lock);
	}
}

struct thread *tm_process_get_head_thread(struct process *proc)
{
	return linkedlist_head(&proc->threadlist);
}

