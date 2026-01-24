#include "ls.h"

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>

Ls* ls_create(void) {
    Ls* ls = (Ls*)malloc(sizeof(Ls));
    if (ls == NULL) {
        return NULL;
    }
    ls->a_ = false;
    ls->l_ = false;
    ls->h_ = false;
    return ls;
}

Ls* ls_create_with_flags(bool a, bool l, bool h) {
    Ls* ls = (Ls*)malloc(sizeof(Ls));
    if (ls == NULL) {
        return NULL;
    }
    ls->a_ = a;
    ls->l_ = l;
    ls->h_ = h;
    return ls;
}

void ls_destroy(Ls* ls) {
    if (ls != NULL) {
        free(ls);
    }
}

void ls_print(const StringMatrix* matrix) {
    if (matrix == NULL || matrix->data == NULL) {
        return;
    }
    
    for (size_t i = 0; i < matrix->rows; ++i) {
        if (matrix->data[i] == NULL) {
            continue;
        }
        for (size_t j = 0; j < matrix->cols[i]; ++j) {
            if (matrix->data[i][j] != NULL) {
                printf("%s", matrix->data[i][j]);
                if (j != matrix->cols[i] - 1) {
                    printf(" ");
                }
            }
        }
        printf("\n");
    }
}

// Helper function to determine file type
static char get_file_type(const char* path) {
    struct stat st;
    if (lstat(path, &st) != 0) {
        return 'f';  // Default to file if stat fails
    }
    
    if (S_ISDIR(st.st_mode)) {
        return 'd';  // Directory
    } else if (S_ISLNK(st.st_mode)) {
        return 'l';  // Symbolic link
    } else {
        return 'f';  // Regular file
    }
}

// Helper function to get file type string for -l flag
static const char* get_file_type_string(char type) {
    switch (type) {
        case 'd':
            return "DIRECTORY";
        case 'l':
            return "SOFTLINK";
        case 'f':
        default:
            return "FILE";
    }
}

static bool is_hidden(const char* name) {
    if (name == NULL || strlen(name) == 0) {
        return false;
    }
    return name[0] == '.';
}

static char* strdup_safe(const char* str) {
    if (str == NULL) {
        return NULL;
    }
    size_t len = strlen(str) + 1;
    char* copy = (char*)malloc(len);
    if (copy != NULL) {
        strcpy(copy, str);
    }
    return copy;
}

LsEntry* ls_scan_directory(Ls* ls, const char* path, size_t* count) {
    // TODO: Implement ls_scan_directory
    if (count == NULL) {
        return NULL;
    }
    
    *count = 0;
    return NULL;
}

StringMatrix* ls_process_entries(Ls* ls, LsEntry* entries, size_t* count) {
    // TODO: Implement ls_process_entries
    StringMatrix* matrix = (StringMatrix*)malloc(sizeof(StringMatrix));
    if (matrix == NULL) {
        return NULL;
    }
    
    matrix->data = NULL;
    matrix->rows = 0;
    matrix->cols = NULL;
    
    return matrix;
}

StringMatrix* ls_run(Ls* ls, const char* path) {
    // TODO: Implement ls_run
    if (ls == NULL || path == NULL) {
        return NULL;
    }
    
    StringMatrix* result = (StringMatrix*)malloc(sizeof(StringMatrix));
    if (result == NULL) {
        return NULL;
    }
    
    result->data = NULL;
    result->rows = 0;
    result->cols = NULL;
    
    ls_print(result);
    return result;
}

void ls_entry_destroy(LsEntry* entry, size_t count) {
    if (entry == NULL) {
        return;
    }
    
    for (size_t i = 0; i < count; ++i) {
        if (entry[i].name != NULL) {
            free(entry[i].name);
        }
    }
    free(entry);
}

void string_matrix_destroy(StringMatrix* matrix) {
    if (matrix == NULL) {
        return;
    }
    
    if (matrix->data != NULL) {
        for (size_t i = 0; i < matrix->rows; ++i) {
            if (matrix->data[i] != NULL) {
                for (size_t j = 0; j < matrix->cols[i]; ++j) {
                    if (matrix->data[i][j] != NULL) {
                        free(matrix->data[i][j]);
                    }
                }
                free(matrix->data[i]);
            }
        }
        free(matrix->data);
    }
    
    if (matrix->cols != NULL) {
        free(matrix->cols);
    }
    
    free(matrix);
}