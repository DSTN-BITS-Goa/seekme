# Lab 1: Take Home

## Usage

### Disk Simulator

To test your disk simulator solution:

```bash
cd PATH/TO/lab1/take-home/Disk_Simulator
python autograde_disk.py
```

### LS Command

To test your ls command solution:

```bash
cd PATH/TO/lab1/take-home/ls_command
mkdir build
cd build
cmake ..
make
make run_tests
```

### Grep

To test your grep solution:

```bash
cd PATH/TO/lab1/take-home/grep
mkdir build
cd build
cmake ..
make run_grep_test
```

To run the eval script for grep:

```bash
cd PATH/TO/lab1/take-home/grep
python autograde_grep.py
```

## Components

This take-home assignment combines components from previous labs and introduces new concepts:

### 1. Disk Simulator (Disk_Simulator/disk.py) - 2 marks

#### Question 1: C-LOOK Algorithm

Modify `Disk_Simulator/disk.py` to implement the C-LOOK algorithm. The set of requests should be able to run with `-p CLOOK` option. In case two requests are on the same track (tie) preserve the original request ordering.

**Example:**
```bash
cd Disk_Simulator
python disk.py -c -a 30,18,13,17,1,16 -w 3 -p CLOOK
```

The requests should be processed in the order: `18,13,30,1,17,16`

**C-LOOK Algorithm:**
- C-LOOK services requests in one direction until no more requests exist in that direction
- Then it jumps to the beginning and continues in the same direction
- Unlike C-SCAN, C-LOOK doesn't go all the way to the end of the disk

#### Question 2: V(R) Algorithm (Non-Evaluative - Self Study)

**Note: V(R) implementation is non-evaluative and is provided for self-study purposes only. It will not be graded.**

Add an implementation of the [V(R) algorithm](https://dl.acm.org/doi/pdf/10.1145/7351.8929) to `Disk_Simulator/disk.py`.

**V(R) Algorithm Explained:**
- V(R) maintains the current direction of the disk head (inwards/outwards)
- V(R) services the next known request with the smallest effective distance
- **Effective distance**:
  1. The effective distance of a request that lies in the current head direction is its physical distance (in tracks) from the current position of the read/write head
  2. The effective distance of a request that does NOT lie in the current head direction is its `Physical distance + R * (Total number of tracks on the disk)`
- `R` is a real number between `0` and `1` (inclusive)
- In case two requests are located on the same track preserve the original request ordering

**Command-line Options:**
- `-p VR` to select the V(R) policy
- `-r R` to specify the R value for the V(R) policy

**Example:**
```bash
cd Disk_Simulator
python disk.py -c -a 5,8,17,57,52,3 -w 3 -n 5 -p VR -r 0.3
```

The requests should be processed in the order: `[5,8,17,3,57,52]`

**Note:**
- For `R = 0`, V(R) behaves identically to `SSTF`
- For `R = 1`, V(R) behaves identically to `LOOK`

Make sure the code works with the various options available.

### 2. LS Command Implementation - 2 marks

Implement a simplified version of the [`ls`](https://man7.org/linux/man-pages/man1/ls.1.html) command in C.

You will be implementing the `Ls` functionality. Refer to the [`ls_command/ls.h`](ls_command/ls.h) file for function declarations and structure definitions.

`ls_run(ls, path)` prints out and returns the contents of the directory specified by `path` in the format specified by the flags.

`ls_print()` has already been implemented for you. Feel free to use this as you see fit.

**StringMatrix** represents a 2-D matrix of `char*` strings.

#### Output

The basic implementation of `ls_run(ls, path)` should return a `StringMatrix` of all files located in `path` with the following format:

```text
relative_file_path_to_file1
relative_file_path_to_file2
...
```

**Note:** The paths expected in [`ls_command/ls_test.c`](ls_command/ls_test.c) are relative to the `build/` directory

#### Flags

##### `-a`

List all entries **including hidden files** (files that start with a `.`). The basic ls command does not list hidden files.

##### `-l`

List in long format. Long format consists of both the **filename and filetype**.

```text
relative_file_path_to_file1 FILE1_TYPE
relative_file_path_to_file2 FILE2_TYPE
...
```

The following `FILE_TYPE`'s are expected:

1. `DIRECTORY` for directories
2. `SOFTLINK` for soft links
3. `FILE` for everything else

##### `-h`

List files that are hardlinked to each other in the directory. Group the hardlinked files together in a single row `StringMatrix`.
The file names in a single row must be sorted in lexicographical order. The rows must also be sorted in lexicographical order amongst themselves.

```text
hardlink1_relative_file_path_to_file1 hardlink1_relative_file_path_to_file2 hardlink1_relative_file_path_to_file3
hardlink2_relative_file_path_to_file4 hardlink2_relative_file_path_to_file5
...
```

###### `-hl`

if `-hl` is supplied as a flag, the output should be in the following format.
Only the file paths should be sorted in lexicographical order and the FILE_TYPE must be the last element in the row

```text
hardlink1_relative_file_path_to_file1 hardlink1_relative_file_path_to_file2 hardlink1_relative_file_path_to_file3 FILE_TYPE
hardlink2_relative_file_path_to_file4 hardlink2_relative_file_path_to_file5 FILE_TYPE
...
```

**Note:** Your implementation will be tested on combinations of flags as well.

**Example:** `ls -ahl` and `ls -lha` would list all files that have more than one hard link including hidden files in long format

**Warning:** Ensure that rows of `StringMatrix` returned by `ls_run()` are arranged in lexicographical order

### 3. Grep Implementation - 2 marks

Implement the file search functionality for a `grep` command.

**What to implement:**

**`grep_search_file()`** - Search pattern in a single file
- Read file line by line
- Match pattern against each line using `grep_match_pattern()` (already implemented)
- Collect matches in `GrepResult` structure
- Handle `-n` flag for line numbers
- Handle `-v` flag for invert match

**Note:** The following utility functions are provided for your use but do not need to be implemented:
- `grep_options_destroy()` - Cleanup function for GrepOptions
- `grep_print_results()` - Print function for search results
- `grep_result_destroy()` - Cleanup function for GrepResult

**Structure:**
- `GrepOptions`: Structure to hold command-line options and flags
- `GrepMatch`: Structure to represent a single match
- `GrepResult`: Structure to hold all matches from a search

Refer to [`grep/grep.h`](grep/grep.h) for detailed function signatures.

## Building and Testing

### Disk Simulator

The disk simulator is a Python script and doesn't require building. To test:

```bash
cd Disk_Simulator
python autograde_disk.py
```

### LS Command

To build and test the ls command:

```bash
cd ls_command
mkdir build
cd build
cmake ..
make
make run_tests
```

Or run the test executable directly:

```bash
./ls_test
```

Or using CTest:

```bash
ctest
```

### Grep

To build and test grep:

```bash
cd grep
mkdir build
cd build
cmake ..
make
```

To run tests:

```bash
make run_grep_test
```

Or using CTest:

```bash
ctest
```

To run the autograder:

```bash
cd grep
python autograde_grep.py
```

## Important Notes

1. All C code should follow C11 standard
2. Proper memory management is critical - free all allocated memory
3. Handle edge cases (NULL pointers, empty directories, etc.)
4. Maintain lexicographical sorting where specified