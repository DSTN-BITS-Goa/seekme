#include "grep.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>

// Destroy GrepOptions and free memory
void grep_options_destroy(GrepOptions* opts) {
    if (opts == NULL) {
        return;
    }
    
    if (opts->pattern != NULL) {
        free(opts->pattern);
    }
    
    if (opts->paths != NULL) {
        for (size_t i = 0; i < opts->path_count; ++i) {
            if (opts->paths[i] != NULL) {
                free(opts->paths[i]);
            }
        }
        free(opts->paths);
    }
    
    free(opts);
}

// Pattern matching function
bool grep_match_pattern(const char* pattern, const char* text, bool case_insensitive) {
    if (pattern == NULL || text == NULL) {
        return false;
    }
    
    if (case_insensitive) {
        // Case-insensitive matching: convert both strings to lowercase for comparison
        size_t pattern_len = strlen(pattern);
        size_t text_len = strlen(text);
        
        if (pattern_len > text_len) {
            return false;
        }
        
        for (size_t i = 0; i <= text_len - pattern_len; i++) {
            bool match = true;
            for (size_t j = 0; j < pattern_len; j++) {
                if (tolower((unsigned char)text[i + j]) != tolower((unsigned char)pattern[j])) {
                    match = false;
                    break;
                }
            }
            if (match) {
                return true;
            }
        }
        return false;
    } else {
        // Case-sensitive matching: use strstr
        return strstr(text, pattern) != NULL;
    }
}

// Search for pattern in a single file
GrepResult* grep_search_file(GrepOptions* opts, const char* filename) {
    // TODO: Implement file search
    // Read file line by line, match pattern against each line, and collect matches
    // Handle line numbers and invert match flags
    
    GrepResult* result = (GrepResult*)malloc(sizeof(GrepResult));
    if (result == NULL) {
        return NULL;
    }
    
    result->matches = NULL;
    result->count = 0;
    result->capacity = 0;
    
    return result;
}

// Print search results
void grep_print_results(GrepResult* result) {
    if (result == NULL || result->matches == NULL) {
        return;
    }
    
    for (size_t i = 0; i < result->count; ++i) {
        GrepMatch* match = &result->matches[i];
        
        if (match->filename != NULL) {
            printf("%s", match->filename);
        }
        
        if (match->line_number > 0) {
            printf(":%d", match->line_number);
        }
        
        if (match->line_content != NULL) {
            printf(":%s", match->line_content);
        }
        
        printf("\n");
    }
}

// Destroy GrepResult and free memory
void grep_result_destroy(GrepResult* result) {
    if (result == NULL) {
        return;
    }
    
    if (result->matches != NULL) {
        for (size_t i = 0; i < result->count; ++i) {
            if (result->matches[i].filename != NULL) {
                free(result->matches[i].filename);
            }
            if (result->matches[i].line_content != NULL) {
                free(result->matches[i].line_content);
            }
        }
        free(result->matches);
    }
    
    free(result);
}