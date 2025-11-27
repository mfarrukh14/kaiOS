/*
 * KaiOS - File System Header
 * Simple in-memory file system with hierarchical directory structure
 */

#ifndef KAIOS_FS_H
#define KAIOS_FS_H

#include "include/kernel/types.h"

// File system constants
#define FS_MAX_NAME       64
#define FS_MAX_PATH       256
#define FS_MAX_FILES      128
#define FS_MAX_FILE_SIZE  65536
#define FS_MAX_CHILDREN   32

// File types
typedef enum {
    FS_FILE = 0,
    FS_DIRECTORY = 1
} fs_type_t;

// File node structure
typedef struct fs_node {
    char name[FS_MAX_NAME];
    fs_type_t type;
    size_t size;
    uint8_t* data;
    struct fs_node* parent;
    struct fs_node* children[FS_MAX_CHILDREN];
    size_t child_count;
    uint32_t created;
    uint32_t modified;
} fs_node_t;

// File system functions
void fs_init(void);
fs_node_t* fs_get_root(void);
fs_node_t* fs_get_current(void);
void fs_set_current(fs_node_t* node);

// Path operations
fs_node_t* fs_resolve_path(const char* path);
int fs_get_path(fs_node_t* node, char* buffer, size_t size);

// Directory operations
fs_node_t* fs_mkdir(const char* name);
int fs_rmdir(const char* name);
int fs_chdir(const char* path);
fs_node_t** fs_readdir(fs_node_t* dir, size_t* count);

// File operations
fs_node_t* fs_create(const char* name);
int fs_delete(const char* name);
int fs_write(fs_node_t* file, const void* data, size_t size);
int fs_append(fs_node_t* file, const void* data, size_t size);
int fs_read(fs_node_t* file, void* buffer, size_t size, size_t offset);
fs_node_t* fs_find(const char* name);

// Utility functions
int fs_rename(const char* old_name, const char* new_name);
int fs_copy(const char* src, const char* dest);

// Persistence functions
bool fs_save(void);
bool fs_load(void);
bool fs_has_disk(void);

#endif // KAIOS_FS_H
