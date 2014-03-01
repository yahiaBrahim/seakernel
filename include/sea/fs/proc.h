#ifndef __SEA_FS_PROC_H
#define __SEA_FS_PROC_H

#include <sea/fs/inode.h>

struct inode *proc_create_node_at_root(char *name, mode_t  mode, int major, int minor);
struct inode *proc_create_node(struct inode *to, char *name, mode_t mode, int major, int minor);
int proc_read(struct inode *i, off_t off, size_t len, char *buffer);
int proc_write(struct inode *i, off_t pos, size_t len, char *buffer);
int proc_append_buffer(char *buffer, char *data, int off, int len, int req_off, int req_len);
void proc_init();

#endif
