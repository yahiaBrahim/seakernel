#ifndef __SEA_DM_BLOCKDEV_H
#define __SEA_DM_BLOCKDEV_H

#include <stdint.h>
#include <sea/tm/kthread.h>
#include <sea/mutex.h>
#include <sea/lib/hash.h>
#include <sea/lib/mpscq.h>

struct blockctl {
	size_t blocksize;
	ssize_t (*rw)(int dir, struct inode *node, uint64_t start, uint8_t *buffer, size_t count);
	int (*select)(struct inode *node, int rw);
	int (*ioctl)(struct inode *node, int cmd, long arg);
	struct kthread elevator;
	struct mutex cachelock;
	struct hash cache;
	struct mpscq queue;
};

struct blockdev {
	uint64_t partbegin;
	size_t partlen;
	struct blockctl *ctl;
};

void blockdev_init(void);
int blockdev_register(struct inode *node, struct blockctl *);
void blockdev_register_partition(struct inode *master, struct inode *part, uint64_t partbegin, uint64_t partlen);
#endif

