/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>
  Copyright (C) 2011       Sebastian Pipping <sebastian@pipping.org>

  This program can be distributed under the terms of the GNU GPLv2.
  See the file COPYING.
*/

#include "params.h"
#include <fuse3/fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>

/* --- data_block init/free --- */
void data_block_init(struct data_block *b, int size)
{
	b->data = (char *)malloc((size_t)size);
	if (b->data)
		memset(b->data, 0, (size_t)size);
}

void data_block_free(struct data_block *b)
{
	free(b->data);
	b->data = NULL;
}

/* --- path_to_inode helpers --- */
void path_to_inode_add(struct myfs_state *s, const char *path, int inode_index)
{
	if (s->path_count >= s->NUM_INODES)
		return;
	strncpy(s->path_to_inode[s->path_count].path, path, PATH_MAX - 1);
	s->path_to_inode[s->path_count].path[PATH_MAX - 1] = '\0';
	s->path_to_inode[s->path_count].inode = inode_index;
	s->path_count++;
}

void path_to_inode_remove(struct myfs_state *s, const char *path)
{
	int i;
	for (i = 0; i < s->path_count; i++) {
		if (strcmp(s->path_to_inode[i].path, path) == 0) {
			/* swap with last */
			if (i != s->path_count - 1) {
				s->path_to_inode[i] = s->path_to_inode[s->path_count - 1];
			}
			s->path_count--;
			return;
		}
	}
}

int path_to_inode_lookup(struct myfs_state *s, const char *path)
{
	int i;
	for (i = 0; i < s->path_count; i++) {
		if (strcmp(s->path_to_inode[i].path, path) == 0)
			return s->path_to_inode[i].inode;
	}
	return -1;
}

/* --- myfs_state create/destroy --- */
struct myfs_state *myfs_state_create(FILE *log, const char *root, int num_inodes,
                                     int num_data_blocks, int data_block_size)
{
	struct myfs_state *s;
	int i;
	char *rootpath;

	s = (struct myfs_state *)malloc(sizeof(struct myfs_state));
	if (!s)
		return NULL;
	s->NUM_INODES = num_inodes;
	s->NUM_DATA_BLOCKS = num_data_blocks;
	s->DATA_BLOCK_SIZE = data_block_size;
	s->logfile = log;
	s->path_count = 0;

	rootpath = realpath(root, NULL);
	if (!rootpath) {
		free(s);
		return NULL;
	}
	s->rootdir = strdup(rootpath);
	free(rootpath);
	if (!s->rootdir) {
		free(s);
		return NULL;
	}

	s->data_blocks = (struct data_block **)malloc((size_t)num_data_blocks * sizeof(struct data_block *));
	if (!s->data_blocks) {
		free(s->rootdir);
		free(s);
		return NULL;
	}
	for (i = 0; i < num_data_blocks; i++) {
		s->data_blocks[i] = (struct data_block *)malloc(sizeof(struct data_block));
		if (!s->data_blocks[i]) {
			while (i--)
				data_block_free(s->data_blocks[i]), free(s->data_blocks[i]);
			free(s->data_blocks);
			free(s->rootdir);
			free(s);
			return NULL;
		}
		data_block_init(s->data_blocks[i], data_block_size);
	}

	s->inodes = (struct inode **)malloc((size_t)num_inodes * sizeof(struct inode *));
	if (!s->inodes) {
		for (i = 0; i < num_data_blocks; i++)
			data_block_free(s->data_blocks[i]), free(s->data_blocks[i]);
		free(s->data_blocks);
		free(s->rootdir);
		free(s);
		return NULL;
	}
	for (i = 0; i < num_inodes; i++) {
		s->inodes[i] = (struct inode *)malloc(sizeof(struct inode));
		if (!s->inodes[i]) {
			while (i--)
				free(s->inodes[i]->blocks), free(s->inodes[i]);
			free(s->inodes);
			for (i = 0; i < num_data_blocks; i++)
				data_block_free(s->data_blocks[i]), free(s->data_blocks[i]);
			free(s->data_blocks);
			free(s->rootdir);
			free(s);
			return NULL;
		}
		s->inodes[i]->num_blocks = 0;
		s->inodes[i]->blocks = (int *)malloc((size_t)num_data_blocks * sizeof(int));
		if (!s->inodes[i]->blocks) {
			free(s->inodes[i]);
			while (i--)
				free(s->inodes[i]->blocks), free(s->inodes[i]);
			free(s->inodes);
			for (i = 0; i < num_data_blocks; i++)
				data_block_free(s->data_blocks[i]), free(s->data_blocks[i]);
			free(s->data_blocks);
			free(s->rootdir);
			free(s);
			return NULL;
		}
	}

	s->inode_bitmap = (int *)calloc((size_t)num_inodes, sizeof(int));
	s->data_block_bitmap = (int *)calloc((size_t)num_data_blocks, sizeof(int));
	if (!s->inode_bitmap || !s->data_block_bitmap) {
		if (s->inode_bitmap) free(s->inode_bitmap);
		if (s->data_block_bitmap) free(s->data_block_bitmap);
		for (i = 0; i < num_inodes; i++)
			free(s->inodes[i]->blocks), free(s->inodes[i]);
		free(s->inodes);
		for (i = 0; i < num_data_blocks; i++)
			data_block_free(s->data_blocks[i]), free(s->data_blocks[i]);
		free(s->data_blocks);
		free(s->rootdir);
		free(s);
		return NULL;
	}

	s->path_to_inode = (struct path_inode *)malloc((size_t)num_inodes * sizeof(struct path_inode));
	if (!s->path_to_inode) {
		free(s->data_block_bitmap);
		free(s->inode_bitmap);
		for (i = 0; i < num_inodes; i++)
			free(s->inodes[i]->blocks), free(s->inodes[i]);
		free(s->inodes);
		for (i = 0; i < num_data_blocks; i++)
			data_block_free(s->data_blocks[i]), free(s->data_blocks[i]);
		free(s->data_blocks);
		free(s->rootdir);
		free(s);
		return NULL;
	}

	return s;
}

void myfs_state_destroy(struct myfs_state *s)
{
	int i;
	if (!s)
		return;
	if (s->logfile)
		fclose(s->logfile);
	free(s->rootdir);
	for (i = 0; i < s->NUM_DATA_BLOCKS; i++) {
		data_block_free(s->data_blocks[i]);
		free(s->data_blocks[i]);
	}
	free(s->data_blocks);
	for (i = 0; i < s->NUM_INODES; i++) {
		free(s->inodes[i]->blocks);
		free(s->inodes[i]);
	}
	free(s->inodes);
	free(s->inode_bitmap);
	free(s->data_block_bitmap);
	free(s->path_to_inode);
	free(s);
}

/* --- logging (DO NOT CHANGE) --- */
FILE *log_open(char *file_name)
{
	FILE *logfile;

	logfile = fopen(file_name, "w");
	if (logfile == NULL) {
		perror("logfile");
		exit(EXIT_FAILURE);
	}
	setvbuf(logfile, NULL, _IOLBF, 0);
	return logfile;
}

void log_char(char c)
{
	FILE *log_file = MYFS_DATA->logfile;
	if (c == '\n')
		fprintf(log_file, "\\n");
	else
		fprintf(log_file, "%c", c);
}

static int path_inode_cmp(const void *a, const void *b)
{
	const struct path_inode *pa = (const struct path_inode *)a;
	const struct path_inode *pb = (const struct path_inode *)b;
	return strcmp(pa->path, pb->path);
}

void log_fuse_context(void)
{
	struct myfs_state *myfs_data = MYFS_DATA;
	FILE *log_file = myfs_data->logfile;
	int i, j, k, num_blocks, block_index;

	if (myfs_data->path_count > 1) {
		qsort(myfs_data->path_to_inode, (size_t)myfs_data->path_count,
		      sizeof(struct path_inode), path_inode_cmp);
	}

	fprintf(log_file, "PATH_TO_INODE_MAP:\n");
	for (i = 0; i < myfs_data->path_count; i++)
		fprintf(log_file, "%s: %d\n",
			myfs_data->path_to_inode[i].path,
			myfs_data->path_to_inode[i].inode);

	fprintf(log_file, "INODE_BITMAP: [");
	for (i = 0; i < myfs_data->NUM_INODES; i++) {
		fprintf(log_file, "%d", myfs_data->inode_bitmap[i]);
		if (i != myfs_data->NUM_INODES - 1)
			fprintf(log_file, ", ");
	}
	fprintf(log_file, "]\n");

	fprintf(log_file, "DATA_BLOCK_BITMAP: [");
	for (i = 0; i < myfs_data->NUM_DATA_BLOCKS; i++) {
		fprintf(log_file, "%d", myfs_data->data_block_bitmap[i]);
		if (i != myfs_data->NUM_DATA_BLOCKS - 1)
			fprintf(log_file, ", ");
	}
	fprintf(log_file, "]\n");

	for (i = 0; i < myfs_data->NUM_INODES; i++) {
		fprintf(log_file, "inode%d: ", i);
		num_blocks = myfs_data->inodes[i]->num_blocks;
		for (j = 0; j < num_blocks; j++) {
			block_index = myfs_data->inodes[i]->blocks[j];
			for (k = 0; k < myfs_data->DATA_BLOCK_SIZE; k++)
				log_char(myfs_data->data_blocks[block_index]->data[k]);
		}
		fprintf(log_file, "\n");
	}
}

void log_msg(const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	vfprintf(MYFS_DATA->logfile, format, ap);
	va_end(ap);
}

/* --- FUSE operations --- */
static void myfs_fullpath(char fpath[PATH_MAX], const char *path)
{
	strcpy(fpath, MYFS_DATA->rootdir);
	strncat(fpath, path, PATH_MAX - 1);
	fpath[PATH_MAX - 1] = '\0';
}

	/* TODO: Implement find_free_inode, find_free_data_block, count_free_data_blocks, allocate_blocks_for_append, and g_inode_logical_size. */

static int myfs_unlink(const char *path)
{
	int res;
	char fpath[PATH_MAX];
	myfs_fullpath(fpath, path);

	log_msg("DELETE %s\n", path);

	/* TODO: Lookup inode, free its data blocks, clear inode and path map, reset logical size. */

	res = unlink(fpath);
	if (res == -1) {
		log_msg("ERROR: DELETE %s\n", path);
		log_fuse_context();
		return -errno;
	}

	log_fuse_context();
	return 0;
}

static int myfs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
	int res;
	char fpath[PATH_MAX];
	myfs_fullpath(fpath, path);

	log_msg("CREATE %s\n", path);

	/* TODO: Find free inode (fail with INODES FULL if none), set bitmap/path map/logical size. */

	res = open(fpath, fi->flags, mode);
	if (res == -1) {
		log_msg("ERROR: CREATE %s\n", path);
		log_fuse_context();
		return -errno;
	}

	fi->fh = (uint64_t)(unsigned long)res;
	log_fuse_context();
	return 0;
}

static int myfs_read(const char *path, char *buf, size_t size, off_t offset,
                     struct fuse_file_info *fi)
{
	int fd;
	ssize_t res;
	char fpath[PATH_MAX];
	myfs_fullpath(fpath, path);

	log_msg("READ %s\n", path);

	/* TODO: Lookup inode, use logical size for total_size, log each block, copy from data blocks to buf. */

	if (fi == NULL)
		fd = open(fpath, O_RDONLY);
	else
		fd = (int)(unsigned long)fi->fh;

	if (fd == -1) {
		log_msg("ERROR: READ %s\n", path);
		log_fuse_context();
		return -errno;
	}

	res = pread(fd, buf, size, offset);
	if (res == -1) {
		log_msg("ERROR: READ %s\n", path);
		if (fi == NULL)
			close(fd);
		return -errno;
	}

	if (fi == NULL)
		close(fd);

	log_fuse_context();
	return (int)res;
}

static int myfs_write(const char *path, const char *buf, size_t size,
                      off_t offset, struct fuse_file_info *fi)
{
	int fd;
	ssize_t res;
	char fpath[PATH_MAX];
	myfs_fullpath(fpath, path);

	log_msg("WRITE %s\n", path);

	/* TODO: Lookup inode; pack (fill last block first), allocate blocks, copy data, update logical size, pwrite. */

	(void)fi;
	if (fi == NULL)
		fd = open(fpath, O_WRONLY);
	else
		fd = (int)(unsigned long)fi->fh;

	if (fd == -1) {
		log_msg("ERROR: WRITE %s\n", path);
		log_fuse_context();
		return -errno;
	}

	res = pwrite(fd, buf, size, offset);
	if (res == -1) {
		log_msg("ERROR: WRITE %s\n", path);
		log_fuse_context();
		if (fi == NULL)
			close(fd);
		return -errno;
	}

	if (fi == NULL)
		close(fd);

	log_fuse_context();
	return (int)res;
}

static void *myfs_init(struct fuse_conn_info *conn, struct fuse_config *cfg)
{
	(void)conn;
	cfg->use_ino = 1;
	cfg->entry_timeout = 0;
	cfg->attr_timeout = 0;
	cfg->negative_timeout = 0;
	/* TODO: Set direct_io and allocate g_inode_logical_size for NUM_INODES. */
	return MYFS_DATA;
}

static int myfs_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi)
{
	int res;
	char fpath[PATH_MAX];
	(void)fi;
	myfs_fullpath(fpath, path);

	res = lstat(fpath, stbuf);
	if (res == -1)
		return -errno;
	return 0;
}

static int myfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                        off_t offset, struct fuse_file_info *fi,
                        enum fuse_readdir_flags flags)
{
	DIR *dp;
	struct dirent *de;
	char fpath[PATH_MAX];

	(void)offset;
	(void)fi;
	(void)flags;
	myfs_fullpath(fpath, path);

	dp = opendir(fpath);
	if (dp == NULL)
		return -errno;

	while ((de = readdir(dp)) != NULL) {
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;
		if (filler(buf, de->d_name, &st, 0, (enum fuse_fill_dir_flags)0))
			break;
	}

	closedir(dp);
	return 0;
}

static int myfs_mkdir(const char *path, mode_t mode)
{
	int res;
	char fpath[PATH_MAX];
	myfs_fullpath(fpath, path);

	res = mkdir(fpath, mode);
	if (res == -1)
		return -errno;
	return 0;
}

static int myfs_rmdir(const char *path)
{
	int res;
	char fpath[PATH_MAX];
	myfs_fullpath(fpath, path);

	res = rmdir(fpath);
	if (res == -1)
		return -errno;
	return 0;
}

static int myfs_open(const char *path, struct fuse_file_info *fi)
{
	int res;
	char fpath[PATH_MAX];
	myfs_fullpath(fpath, path);

	res = open(fpath, fi->flags);
	if (res == -1)
		return -errno;
	fi->fh = (uint64_t)(unsigned long)res;
	return 0;
}

static int myfs_release(const char *path, struct fuse_file_info *fi)
{
	(void)path;
	close((int)(unsigned long)fi->fh);
	return 0;
}

static const struct fuse_operations myfs_oper = {
	.getattr  = myfs_getattr,
	.mkdir    = myfs_mkdir,
	.unlink   = myfs_unlink,
	.rmdir    = myfs_rmdir,
	.open     = myfs_open,
	.read     = myfs_read,
	.write    = myfs_write,
	.release  = myfs_release,
	.readdir  = myfs_readdir,
	.init     = myfs_init,
	.create   = myfs_create,
};

void myfs_usage(void)
{
	fprintf(stderr, "usage:  myfs [FUSE and mount options] mount_point log_file root_dir num_inodes num_data_blocks data_block_size\n");
	abort();
}

int main(int argc, char *argv[])
{
	int fuse_stat;
	struct myfs_state *myfs_data;
	FILE *logf;

	if ((getuid() == 0) || (geteuid() == 0)) {
		fprintf(stderr, "Running BBFS as root opens unnacceptable security holes\n");
		return 1;
	}

	fprintf(stderr, "Fuse library version %d.%d\n", FUSE_MAJOR_VERSION, FUSE_MINOR_VERSION);

	if ((argc < 6) || (argv[argc - 6][0] == '-') || (argv[argc - 5][0] == '-') || (argv[argc - 4][0] == '-'))
		myfs_usage();

	logf = log_open(argv[argc - 5]);
	myfs_data = myfs_state_create(logf, argv[argc - 4],
	                              atoi(argv[argc - 3]), atoi(argv[argc - 2]), atoi(argv[argc - 1]));
	if (!myfs_data) {
		fclose(logf);
		fprintf(stderr, "myfs_state_create failed\n");
		return 1;
	}

	argc -= 5;

	fprintf(stderr, "about to call fuse_main\n");
	fuse_stat = fuse_main(argc, argv, &myfs_oper, myfs_data);
	fprintf(stderr, "fuse_main returned %d\n", fuse_stat);

	myfs_state_destroy(myfs_data);
	return fuse_stat;
}
