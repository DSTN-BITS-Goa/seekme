#ifndef GREP_H
#define GREP_H

#include <stdbool.h>
#include <stdlib.h>

// GrepOptions structure to hold command-line options
typedef struct {
    char* pattern;          // Search pattern
    bool recursive;         // -r flag: recursive search
    bool case_insensitive;  // -i flag: case-insensitive search
    bool line_number;       // -n flag: print line numbers
    bool invert_match;      // -v flag: invert match
    char** paths;           // Paths to search (can be multiple)
    size_t path_count;      // Number of paths
} GrepOptions;

// GrepMatch structure to hold a single match
typedef struct {
    char* filename;     // File where match was found
    int line_number;    // Line number (if -n flag is used)
    char* line_content; // Content of the matching line
} GrepMatch;

// GrepResult structure to hold all matches
typedef struct {
    GrepMatch* matches;  // Array of matches
    size_t count;        // Number of matches
    size_t capacity;     // Capacity of matches array
} GrepResult;

// Function declarations

// Destroy GrepOptions and free memory
void grep_options_destroy(GrepOptions* opts);

// NOTE: To be implemented in grep.c
// Search for pattern in a single file
GrepResult* grep_search_file(GrepOptions* opts, const char* filename);

// Print search results
void grep_print_results(GrepResult* result);

// Destroy GrepResult and free memory
void grep_result_destroy(GrepResult* result);

// Pattern matching function
bool grep_match_pattern(const char* pattern, const char* text, bool case_insensitive);

#endif  // GREP_H