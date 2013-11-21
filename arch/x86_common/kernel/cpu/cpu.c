#include <kernel.h>
#include <cpu.h>

int probe_smp();

void cpuid_get_features(cpuid_t *cpuid)
{
	int eax, ebx, ecx, edx;
	eax = 0x01;
	asm("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(eax));
	/* if no LAPIC is present, the INIT IPI will fail. */
	cpuid->features_edx = edx;
	cpuid->features_ecx = ecx;
	cpuid->stepping =    (eax & 0x0F);
	cpuid->model =       ((eax >> 4) & 0x0F);
	cpuid->family =      ((eax >> 8) & 0x0F);
	cpuid->type =        ((eax >> 12) & 0x03);
	cpuid->cache_line_size =     ((ebx >> 8) & 0xFF) * 8; /* cache_line_size * 8 = size in bytes */
	cpuid->logical_processors =  ((ebx >> 16) & 0xFF);    /* # logical cpu's per physical cpu */
	cpuid->lapic_id =            ((ebx >> 24) & 0xFF);    /* Local APIC ID */
	eax = 0x80000001;
	asm("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(eax));
	cpuid->ext_features_edx = edx;
	cpuid->ext_features_ecx = ecx;
} 

void cpuid_get_cpu_brand(cpuid_t *cpuid)
{
	int eax, ebx, ecx, edx;
	eax = 0x80000002;
	asm("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(eax));
	cpuid->cpu_brand[48] = 0;     /* init cpu_brand to null-terminate the string */
	char *ccb = cpuid->cpu_brand;
	*(int*)(ccb + 0 ) = (int)eax;
	*(int*)(ccb + 4 ) = ebx;
	*(int*)(ccb + 8 ) = ecx;
	*(int*)(ccb + 12) = edx;
	eax = 0x80000003;
	asm("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(eax));
	*(int*)(ccb + 16) = eax;
	*(int*)(ccb + 20) = ebx;
	*(int*)(ccb + 24) = ecx;
	*(int*)(ccb + 28) = edx;
	eax = 0x80000004;
	asm("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(eax));
	*(int*)(cpuid->cpu_brand + 32) = eax;
	*(int*)(cpuid->cpu_brand + 36) = ebx;
	*(int*)(cpuid->cpu_brand + 40) = ecx;
	*(int*)(cpuid->cpu_brand + 44) = edx;
	printk(KERN_DEBUG, "\tCPU Brand: %s \n", cpuid->cpu_brand);
}

void parse_cpuid(cpu_t *me)
{
	cpuid_t cpuid;
	int eax, ebx, ecx, edx;
	eax = 0x00;
	asm("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(eax));
	cpuid.max_basic_input_val = eax;
	memset(cpuid.manufacturer_string, 0, 13);
	char *ccb = cpuid.manufacturer_string;
	*(int*)(ccb + 0) = ebx;
	*(int*)(cpuid.manufacturer_string + 4) = edx;
	*(int*)(cpuid.manufacturer_string + 8) = ecx;
	printk(KERN_DEBUG, "[cpu]: CPUID: ");
	printk(KERN_DEBUG, "%s\n", cpuid.manufacturer_string);
	if(cpuid.max_basic_input_val >= 1)
		cpuid_get_features(&cpuid);
	eax = 0x80000000;
	asm("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(eax));
	cpuid.max_ext_input_val = eax; 
	if((unsigned int)cpuid.max_ext_input_val >= 0x80000004)
		cpuid_get_cpu_brand(&cpuid);
	memcpy(&(me->cpuid), &cpuid, sizeof(me->cpuid));
}
#if CONFIG_SMP

cpu_t *get_cpu(int id)
{
	for(unsigned int i=0;i<cpu_array_num;i++)
	{
		if(cpu_array[i].apicid == id) return &cpu_array[i];
	}
	return 0;
}

cpu_t *add_cpu(cpu_t *c)
{
	if(cpu_array_num >= CONFIG_MAX_CPUS)
		return 0;
	memcpy(&cpu_array[cpu_array_num], c, sizeof(cpu_t));
	mutex_create((mutex_t *)&(cpu_array[cpu_array_num].lock), MT_NOSCHED);
	return &cpu_array[cpu_array_num++];
}

int probe_smp();
#endif

int set_int(unsigned _new)
{
	/* need to make sure we don't get interrupted... */
	asm("cli");
	cpu_t *cpu = current_task ? current_task->cpu : 0;
	unsigned old = cpu ? cpu->flags&CPU_INTER : 0;
	if(!_new) {
		asm("cli");
		if(cpu) cpu->flags &= ~CPU_INTER;
	} else if(!cpu || cpu->flags&CPU_RUNNING) {
		asm("sti");
		if(cpu) cpu->flags |= CPU_INTER;
	}
	return old;
}

void set_cpu_interrupt_flag(int flag)
{
	cpu_t *cpu = current_task ? current_task->cpu : 0;
	if(!cpu) return;
	if(flag)
		cpu->flags |= CPU_INTER;
	else
		cpu->flags &= ~CPU_INTER;
}

int get_cpu_interrupt_flag()
{
	cpu_t *cpu = current_task ? current_task->cpu : 0;
	return (cpu ? (cpu->flags&CPU_INTER) : 0);
}
