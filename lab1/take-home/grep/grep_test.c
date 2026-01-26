#include "grep.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]) {
    // Create test file
    const char* test_file = "grep_test_file.txt";
    FILE* f = fopen(test_file, "w");
    if (f == NULL) {
        fprintf(stderr, "Failed to create test file\n");
        return 1;
    }
    
    fprintf(f, "hello world\n");
    fprintf(f, "this is a test\n");
    fprintf(f, "Hello World\n");
    fprintf(f, "another line\n");
    fprintf(f, "test pattern here\n");
    fprintf(f, "no match line\n");
    fclose(f);
    
    // Create GrepOptions
    GrepOptions* opts = (GrepOptions*)malloc(sizeof(GrepOptions));
    if (opts == NULL) {
        fprintf(stderr, "Failed to allocate GrepOptions\n");
        return 1;
    }
    
    opts->pattern = strdup("test");
    opts->recursive = false;
    opts->case_insensitive = false;
    opts->line_number = true;
    opts->invert_match = false;
    opts->paths = NULL;
    opts->path_count = 0;
    
    printf("=== Test 1: Basic pattern matching with line numbers ===\n");
    printf("Pattern: 'test'\n");
    printf("Expected matches: lines 2, 5\n\n");
    
    // Test basic search
    GrepResult* result = grep_search_file(opts, test_file);
    if (result == NULL) {
        fprintf(stderr, "grep_search_file returned NULL\n");
        grep_options_destroy(opts);
        return 1;
    }
    
    printf("Found %zu matches:\n", result->count);
    grep_print_results(result);
    printf("\n");
    
    if (result->count != 2) {
        printf("ERROR: Expected 2 matches, got %zu\n", result->count);
    } else {
        printf("PASS: Found correct number of matches\n");
    }
    
    grep_result_destroy(result);
    
    // Test 2: Case-insensitive search
    printf("\n=== Test 2: Case-insensitive search ===\n");
    opts->case_insensitive = true;
    opts->pattern = "hello";
    printf("Pattern: 'hello' (case-insensitive)\n");
    printf("Expected matches: lines 1, 3\n\n");
    
    result = grep_search_file(opts, test_file);
    if (result == NULL) {
        fprintf(stderr, "grep_search_file returned NULL\n");
        grep_options_destroy(opts);
        return 1;
    }
    
    printf("Found %zu matches:\n", result->count);
    grep_print_results(result);
    printf("\n");
    
    if (result->count != 2) {
        printf("ERROR: Expected 2 matches, got %zu\n", result->count);
    } else {
        printf("PASS: Found correct number of matches\n");
    }
    
    grep_result_destroy(result);
    
    // Test 3: Invert match
    printf("\n=== Test 3: Invert match (-v flag) ===\n");
    opts->invert_match = true;
    opts->pattern = "test";
    opts->case_insensitive = false;
    printf("Pattern: 'test' (inverted)\n");
    printf("Expected matches: lines 1, 3, 4, 6 (all lines NOT containing 'test')\n\n");
    
    result = grep_search_file(opts, test_file);
    if (result == NULL) {
        fprintf(stderr, "grep_search_file returned NULL\n");
        grep_options_destroy(opts);
        return 1;
    }
    
    printf("Found %zu matches:\n", result->count);
    grep_print_results(result);
    printf("\n");
    
    if (result->count != 4) {
        printf("ERROR: Expected 4 matches, got %zu\n", result->count);
    } else {
        printf("PASS: Found correct number of matches\n");
    }
    
    grep_result_destroy(result);
    
    // Test 4: Without line numbers
    printf("\n=== Test 4: Search without line numbers ===\n");
    opts->invert_match = false;
    opts->line_number = false;
    opts->pattern = "test";
    printf("Pattern: 'test' (no line numbers)\n\n");
    
    result = grep_search_file(opts, test_file);
    if (result == NULL) {
        fprintf(stderr, "grep_search_file returned NULL\n");
        grep_options_destroy(opts);
        return 1;
    }
    
    printf("Found %zu matches:\n", result->count);
    grep_print_results(result);
    printf("\n");
    
    if (result->count != 2) {
        printf("ERROR: Expected 2 matches, got %zu\n", result->count);
    } else {
        printf("PASS: Found correct number of matches\n");
    }
    
    grep_result_destroy(result);
    
    // Cleanup
    grep_options_destroy(opts);
    
    printf("\n=== All tests completed ===\n");
    
    return 0;
}
