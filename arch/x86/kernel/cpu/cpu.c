#include <types.h>
#include <kernel.h>
#include <cpu.h>
#include <cpu-x86.h>
#include <task.h>
#include <mutex.h>
#include <atomic.h>
#include <symbol.h>
#include <acpi.h>
cpu_t *primary_cpu=0;
#if CONFIG_SMP
cpu_t cpu_array[CONFIG_MAX_CPUS];
unsigned cpu_array_num=0;
#endif
cpu_t primary_cpu_data;
void init_lapic(int);
void set_debug_traps (void);
int probe_smp();

void init_main_cpu_1()
{
#if CONFIG_SMP
	mutex_create(&ipi_mutex, MT_NOSCHED);
	memset(cpu_array, 0, sizeof(cpu_t) * CONFIG_MAX_CPUS);
	cpu_array_num = 0;
	int res = probe_smp();
	if(!(kernel_state_flags & KSF_CPUS_RUNNING))
		primary_cpu = &cpu_array[0];
	if(!primary_cpu)
		primary_cpu = &primary_cpu_data;
	load_tables_ap(primary_cpu);
	init_lapic(1);
	calibrate_lapic_timer(1000);
	init_ioapic();
 	if(res >= 0) {
		set_ksf(KSF_SMP_ENABLE);
	} else
		kprintf("[smp]: error in init code, disabling SMP support\n");
#else
	primary_cpu = &primary_cpu_data;
	memset(primary_cpu, 0, sizeof(cpu_t));
	load_tables_ap(primary_cpu);
#endif
	assert(primary_cpu);
	set_int(0);
	primary_cpu->flags = CPU_UP;
	printk(KERN_MSG, "Initializing CPU...\n");
	parse_cpuid(primary_cpu);
	setup_fpu(primary_cpu);
	init_sse(primary_cpu);
	primary_cpu->flags |= CPU_RUNNING;
	printk(KERN_EVERY, "done\n");
	mutex_create((mutex_t *)&primary_cpu->lock, MT_NOSCHED);
#if CONFIG_MODULES
	_add_kernel_symbol((unsigned)(cpu_t *)primary_cpu, "primary_cpu");
	add_kernel_symbol(set_int);
#if CONFIG_SMP
	add_kernel_symbol(get_cpu);
	add_kernel_symbol((addr_t)&cpu_array_num);
	add_kernel_symbol((addr_t)&num_booted_cpus);
#endif
#endif

#if CONFIG_GDB_STUB
	set_debug_traps();
	kprintf("---[DEBUG - waiting for GDB connection]---\n");
	__asm__("int $3");
	kprintf("---[DEBUG - resuming]---\n");
#endif
}

void init_main_cpu_2()
{
	init_acpi();
}
