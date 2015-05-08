#ifndef __SEA_FS_INITRD_H
#define __SEA_FS_INITRD_H

#include <sea/boot/multiboot.h>
#include <sea/types.h>

struct ustar_header {
	char name[100];
	char mode[8];
	char uid[8];
	char gid[8];
	char size[12];
	char mtime[12];
	char checksum[8];
	char typeflag[1];
	char linkname[100];
	char magic[6];
	char version[2];
	char uname[32];
	char gname[32];
	char devmajor[8];
	char devminor[8];
	char prefix[155];
	char pad[12];
};

void fs_initrd_load(struct multiboot *mb);
void fs_initrd_parse();

#endif
