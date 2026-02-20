# Lab 2: Take Home

## Known Issues

- In `myfs_write` the buffer being passed is not consistent all the time. Use `size` to get the size of the buffer. If you try to manually calculate it you may get different values depending on the buffer passed at that time.

## Usage

To test your solution:

```bash
    # Creates an executable named `myfs`
    cd PATH/TO/lab2/take-home
    mkdir build
    cd build
    cmake ..
    make run_tests
```
- This will create a `mount_tc{i}` and `root_tc{i}` folder for all the testcases in the `build` directory and then run each testcase on their respective folders
- The logs for each testcase will be stored in `logs/myfs_tc{i}.log` which you can view
- After the test is done the `mount_tc{i}` and `root_tc{i}` folders will be unmounted and deleted

Note: make sure after you run the tests that `mount_tc{i}` and `root_tc{i}` folders are unmounted and deleted. The Makefile should automatically handle it, but in case of any errors manually make these changes
```bash
    # Unmount and delete the folders
    fusermount -u mount_tc{i}
    rm -rf mount_tc{i} root_tc{i}
```

## Background

In this lab you will be exploring the basics of [FUSE](https://github.com/libfuse/libfuse)

- First install the latest version (3.16.2) of FUSE on your system
- Go through the basics of FUSE and what it is used for
- Read the descriptions of the file system operations it [supports](https://github.com/libfuse/libfuse/blob/master/include/fuse.h#L317)
- A few tutorials to help you understand FUSE
  - https://www.cs.nmsu.edu/~pfeiffer/fuse-tutorial/
  - https://maastaar.net/fuse/linux/filesystem/c/2016/05/21/writing-a-simple-filesystem-using-fuse/
  - [FUSE Video demo](https://www.youtube.com/watch?v=aMlX2x5N9Ak)

- to install fuse3 on ubuntu
```bash
    sudo apt-get install fuse3 libfuse3-dev
```

## Starter code

- You are given a starter implementation of a simple FUSE file system that mirrors a given folder
- The implementation handles creating/reading/writing/deleting files and directories
- The `fuse_context`'s `private_data` points to a single `struct myfs_state` (use the `MYFS_DATA` macro to access it). It contains:
  - `logfile`: A file pointer to the log file
  - `rootdir`: Path to the root directory being mirrored
  - `NUM_INODES`: The number of inodes in the file system
  - `NUM_DATA_BLOCKS`: The number of data blocks in the file system
  - `DATA_BLOCK_SIZE`: The size of each data block
  - `inodes`: An array of pointers to inodes (see `params.h`)
    - Each inode has `blocks` (array of data block indices) and `num_blocks`
  - `data_blocks`: An array of pointers to data blocks
    - Each data block has `data` (a buffer of size `DATA_BLOCK_SIZE`) that stores file data
  - `inode_bitmap`: A bitmap (array of int) for free/allocated status of inodes
  - `data_block_bitmap`: A bitmap (array of int) for free/allocated status of data blocks
  - `path_to_inode`: An array of path-to-inode entries; `path_count` is the number of entries. Use `path_to_inode_add()` when creating a file and `path_to_inode_remove()` when unlinking. Use `path_to_inode_lookup()` to get the inode index for a path.

- In `myfs_init` you must set `direct_io` and allocate a per-inode logical size array (e.g. `g_inode_logical_size`) of length `NUM_INODES` so that read/write/unlink can track file size independently of the underlying mirror.

- Additionally some helper functions have been given
  - `log_fuse_context`: Logs the contents of the fuse_context
  - `log_msg`: Logs a message to the log file
    - This function is very useful for debugging any errors
    - Example usage: `log_msg("Reading Inode %d for file %s\n", i, path)`
  - `log_char`: Use when logging data being read (e.g. in myfs_read)

- Usage:
```bash
    myfs [FUSE and mount options] mount_point log_file root_dir num_inodes num_data_blocks data_block_size
```

## Question

- Modify the read/write/create/delete operations in `myfs.c` to keep track of inodes and data blocks.

### Inode and data block tracking (create, read, append write, unlink)

- **`myfs_create`**
  - Add code to allocate an inode and data block for the new file
  - Use the inode_bitmap and data_block_bitmap to get the free inode and data block
    - If there are no free inodes or data blocks, `log_msg("ERROR: INODES FULL\n")` and return -1 (don't perform the create)
  - If there are multiple inodes/data blocks free, use the block with the lowest index
    - For example, if inodes 2, 4, 7 are free, use inode 2

- **`myfs_read`**
  - Use the file's logical size (and offset/size) to determine what to read; read from the inode's data blocks
  - Log the data blocks being read along with the data itself like so
    ```txt
    DATA BLOCK 0: com
    DATA BLOCK 1: e!W\n
    ```
  - Use the `log_char()` function when logging data being read

- **`myfs_write`**
  - Write the data to the data blocks of the file; when appending, fill the last block first then allocate new blocks as needed
  - When allocating blocks choose blocks from the lowest index onwards
    - For example if you need 2 datablocks and blocks 2, 4, 7 are free, use blocks 2 and 4
  - If there are not enough data blocks `log_msg("ERROR: NOT ENOUGH DATA BLOCKS\n")` and return -1 (don't perform the write)

- **`myfs_unlink`**
  - Look up the inode for the path, free its data blocks, clear the inode and path map, and reset the file's logical size

### General requirements

- Ensure that for each of the 4 operations (`myfs_create`, `myfs_read`, `myfs_write`, `myfs_unlink`) `log_fuse_context` is called regardless of whether it succeeded or failed, and it should be called after any other logs specified above (Refer to `expected_logs` for more clarity).
- On a sidenote, strongly recommend going through https://www.cs.nmsu.edu/~pfeiffer/fuse-tutorial/ which covers many things that are not well documented about FUSE.

### Test Cases

- All test cases are visible in `test.py`. They are invoked via cmake and logs generated by your implementation are verified against logs in `expected_logs/`