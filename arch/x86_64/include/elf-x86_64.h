#ifndef ELF_x86_64_H
#define ELF_x86_64_H
#include <mod.h>
#include <block.h>
#define MAX_SYMS 512
extern void * kernel_start;
/* TODO: Separate into arch-dependant things */
typedef struct { 
	const char *name; 
	intptr_t ptr; 
	int flag;
} kernel_symbol_t;

typedef struct __attribute__((packed))
{
	uint8_t  id[16];
	uint16_t type;
	uint16_t machine;
	uint32_t version;
	uint32_t entry;
	uint32_t phoff;
	uint32_t shoff;
	uint32_t flags;
	uint16_t size;
	uint16_t phsize;
	uint16_t phnum;
	uint16_t shsize;
	uint16_t shnum;
	uint16_t strndx;
	char *shbuf;
	unsigned strtab_addr, symtab_addr, strtabsz, syment_len;
} elf32_header_t;

typedef struct __attribute__((packed))
{
	uint8_t  id[16];
	uint16_t type;
	uint16_t machine;
	uint32_t version;
	addr_t entry;
	uint64_t phoff;
	uint64_t shoff;
	uint32_t flags;
	uint16_t size;
	uint16_t phsize;
	uint16_t phnum;
	uint16_t shsize;
	uint16_t shnum;
	uint16_t strndx;
	/* these aren't....real. */
	char *shbuf;
	unsigned strtab_addr, symtab_addr, strtabsz, syment_len;
} elf64_header_t;

typedef elf64_header_t elf_header_t;

typedef struct __attribute__((packed))
{
	uint32_t name;
	uint32_t type;
	uint32_t flags;
	uint32_t address;
	uint32_t offset;
	uint32_t size;
	uint32_t link;
	uint32_t info;
	uint32_t alignment;
	uint32_t sect_size;
} elf32_section_header_t;

typedef struct __attribute__((packed))
{
	uint32_t name;
	uint32_t type;
	uint64_t flags;
	addr_t address;
	uint64_t offset;
	uint64_t size;
	uint32_t link;
	uint32_t info;
	uint64_t alignment;
	uint64_t sect_size;
} elf64_section_header_t;

typedef struct __attribute__((packed)) {
	uint64_t offset;
	uint64_t info;
} elf64_rel_t;

typedef struct __attribute__((packed))
{
	uint32_t name;
	uint32_t address;
	uint32_t size;
	uint8_t  info;
	uint8_t  other;
	uint16_t shndx;
} elf32_symtab_entry_t;

typedef struct __attribute__((packed))
{
	uint32_t name;
	uint8_t  info;
	uint8_t  other;
	uint16_t shndx;
	addr_t address;
	uint64_t size;
} elf64_symtab_entry_t;

typedef struct __attribute__((packed))
{
	uint32_t p_type;
	uint32_t p_flags;
	uint64_t p_offset;
	addr_t p_addr;
	addr_t p_paddr;
	uint64_t p_filesz;
	uint64_t p_memsz;
	uint64_t p_align;
} elf64_program_header_t;

static inline int is_valid_elf(char *buf, short type)
{
	elf64_header_t * eh;
	eh = (elf64_header_t*)buf;
	if(memcmp(eh->id + 1, (uint8_t*)"ELF", 3)
		|| eh->machine != 0x03
		|| eh->type != type
		|| eh->id[4] != 2 /* 64 bit */)
		return 0;
	return 1;
}

#define PH_LOAD    1
#define PH_DYNAMIC 2
#define PH_INTERP  3


#define EDT_NEEDED   1
#define EDT_PLTRELSZ 2
#define EDT_STRTAB   5
#define EDT_SYMTAB   6

#define EDT_STRSZ   10
#define EDT_SYMENT  11

#define EDT_REL     17
#define EDT_RELSZ   18
#define EDT_RELENT  19
#define EDT_PLTREL  20

#define EDT_TEXTREL 22
#define EDT_JMPREL  23

#define ELF_ST_TYPE(i) ((i)&0xf)
typedef struct {
	uint64_t d_tag;
	union {
		uint64_t d_val;
		addr_t d_ptr;
	} d_un;
} elf64_dyn_t;

typedef struct
{
	elf32_symtab_entry_t *symtab;
	uint32_t      symtabsz;
	const char   *strtab;
	uint32_t      strtabsz;
	unsigned lookable;
} elf32_t;

typedef struct
{
	elf64_symtab_entry_t *symtab;
	uint64_t      symtabsz;
	const char   *strtab;
	uint64_t      strtabsz;
	unsigned lookable;
} elf64_t;

#define GET_RELOC_SYM(i)  ((i)>>8)
#define GET_RELOC_TYPE(i) ((unsigned char)(i))

#define GET_SYMTAB_BIND(i)   ((i)>>4)
#define GET_SYMTAB_TYPE(i)   ((i)&0xf)

#define SHN_UNDEF   0

int parse_elf_module(module_t *mod, uint8_t * buf, char *name, int);
const char *elf64_lookup_symbol (uint64_t addr, elf64_t *elf);
elf32_t parse_kernel_elf(struct multiboot *mb, elf32_t *);
extern elf32_t kernel_elf;
const char *elf64_lookup_symbol (uint64_t addr, elf64_t *elf);
elf64_symtab_entry_t * fill_symbol_struct(uint8_t * buf, uint64_t symbol);
intptr_t get_section_offset(uint8_t * buf, uint64_t info);
char *get_symbol_string(uint8_t *buf, uint64_t index);
int process_elf(char *mem, int fp, addr_t *start, addr_t *end);
unsigned long long get_epoch_time();
void remove_dfs_node(char *name);
int kernel_cache_sync_slow(int all);
int sys_setsid();
int proc_set_callback(int major, int( *callback)(char rw, struct inode *inode, 
												 int m, char *buf));
int proc_get_major();
void delay_sleep(int t);
#endif
