#ifndef _PARAMS_H_
#define _PARAMS_H_

#define FUSE_USE_VERSION 31

#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <limits.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

/* DO NOT CHANGE THIS STRUCT */
struct inode {
	/* indices of data blocks associated with this inode */
	int *blocks;
	int num_blocks;
};

/* DO NOT CHANGE THIS STRUCT */
struct data_block {
	char *data;
};

/* One path-to-inode mapping entry (path_count <= NUM_INODES) */
struct path_inode {
	char path[PATH_MAX];
	int inode;
};

/* DO NOT CHANGE THIS STRUCT */
struct myfs_state {
	int NUM_DATA_BLOCKS;
	int NUM_INODES;
	int DATA_BLOCK_SIZE;

	FILE *logfile;
	char *rootdir;

	struct data_block **data_blocks;
	struct inode **inodes;

	int *inode_bitmap;
	int *data_block_bitmap;

	struct path_inode *path_to_inode;
	int path_count;
};

/* Initialize a data block; size = DATA_BLOCK_SIZE */
void data_block_init(struct data_block *b, int size);

/* Free a data block */
void data_block_free(struct data_block *b);

/* Create and initialize myfs_state; returns NULL on failure */
struct myfs_state *myfs_state_create(FILE *log, const char *root, int num_inodes,
                                      int num_data_blocks, int data_block_size);

/* Free myfs_state and all owned resources */
void myfs_state_destroy(struct myfs_state *s);

/* Add (path, inode_index) to path_to_inode; use when creating a file */
void path_to_inode_add(struct myfs_state *s, const char *path, int inode_index);

/* Remove entry for path; use when unlinking a file */
void path_to_inode_remove(struct myfs_state *s, const char *path);

/* Lookup inode index for path; returns -1 if not found */
int path_to_inode_lookup(struct myfs_state *s, const char *path);

#define MYFS_DATA ((struct myfs_state *) fuse_get_context()->private_data)

#endif
