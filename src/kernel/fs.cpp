/*
 * KaiOS - File System Implementation
 * Simple in-memory file system with disk persistence
 */

#include "include/kernel/fs.h"
#include "include/kernel/memory.h"
#include "include/kernel/string.h"
#include "include/drivers/ata.h"

// Root directory and current directory
static fs_node_t* root_dir = NULL;
static fs_node_t* current_dir = NULL;

// Simple tick counter for timestamps
static uint32_t fs_time = 0;

static uint32_t get_time(void) {
    return ++fs_time;
}

void fs_init(void) {
    // Create root directory
    root_dir = (fs_node_t*)kmalloc(sizeof(fs_node_t));
    if (root_dir == NULL) return;
    
    memset(root_dir, 0, sizeof(fs_node_t));
    strcpy(root_dir->name, "/");
    root_dir->type = FS_DIRECTORY;
    root_dir->parent = root_dir;  // Root's parent is itself
    root_dir->child_count = 0;
    root_dir->created = get_time();
    root_dir->modified = root_dir->created;
    
    current_dir = root_dir;
    
    // Create some default directories
    fs_mkdir("bin");
    fs_mkdir("etc");
    fs_mkdir("home");
    fs_mkdir("tmp");
    
    // Change to home directory
    fs_chdir("home");
    fs_mkdir("user");
    fs_chdir("/");
}

fs_node_t* fs_get_root(void) {
    return root_dir;
}

fs_node_t* fs_get_current(void) {
    return current_dir;
}

void fs_set_current(fs_node_t* node) {
    if (node != NULL && node->type == FS_DIRECTORY) {
        current_dir = node;
    }
}

// Find child by name in a directory
static fs_node_t* find_child(fs_node_t* dir, const char* name) {
    if (dir == NULL || name == NULL || dir->type != FS_DIRECTORY) {
        return NULL;
    }
    
    for (size_t i = 0; i < dir->child_count; i++) {
        if (strcmp(dir->children[i]->name, name) == 0) {
            return dir->children[i];
        }
    }
    
    return NULL;
}

fs_node_t* fs_resolve_path(const char* path) {
    if (path == NULL || *path == '\0') {
        return current_dir;
    }
    
    fs_node_t* node;
    char path_copy[FS_MAX_PATH];
    char* token;
    char* rest;
    
    strncpy(path_copy, path, FS_MAX_PATH - 1);
    path_copy[FS_MAX_PATH - 1] = '\0';
    
    // Absolute or relative path
    if (path_copy[0] == '/') {
        node = root_dir;
        rest = path_copy + 1;
    } else {
        node = current_dir;
        rest = path_copy;
    }
    
    // Parse path components
    token = rest;
    while (*token != '\0') {
        // Find next separator
        char* sep = strchr(token, '/');
        if (sep != NULL) {
            *sep = '\0';
        }
        
        if (strlen(token) > 0) {
            if (strcmp(token, ".") == 0) {
                // Current directory - do nothing
            } else if (strcmp(token, "..") == 0) {
                // Parent directory
                node = node->parent;
            } else {
                // Find child
                fs_node_t* child = find_child(node, token);
                if (child == NULL) {
                    return NULL;  // Path not found
                }
                node = child;
            }
        }
        
        if (sep != NULL) {
            token = sep + 1;
        } else {
            break;
        }
    }
    
    return node;
}

int fs_get_path(fs_node_t* node, char* buffer, size_t size) {
    if (node == NULL || buffer == NULL || size == 0) {
        return -1;
    }
    
    if (node == root_dir) {
        if (size < 2) return -1;
        buffer[0] = '/';
        buffer[1] = '\0';
        return 0;
    }
    
    // Build path from node to root
    char temp[FS_MAX_PATH];
    char* parts[64];
    int count = 0;
    
    fs_node_t* current = node;
    while (current != root_dir && count < 64) {
        parts[count++] = current->name;
        current = current->parent;
    }
    
    // Construct path
    buffer[0] = '\0';
    for (int i = count - 1; i >= 0; i--) {
        strcat(buffer, "/");
        strcat(buffer, parts[i]);
    }
    
    if (buffer[0] == '\0') {
        strcpy(buffer, "/");
    }
    
    return 0;
}

fs_node_t* fs_mkdir(const char* name) {
    if (name == NULL || strlen(name) == 0 || strlen(name) >= FS_MAX_NAME) {
        return NULL;
    }
    
    if (strchr(name, '/') != NULL) {
        return NULL;  // No slashes in name
    }
    
    if (current_dir->child_count >= FS_MAX_CHILDREN) {
        return NULL;  // Directory full
    }
    
    if (find_child(current_dir, name) != NULL) {
        return NULL;  // Already exists
    }
    
    fs_node_t* dir = (fs_node_t*)kmalloc(sizeof(fs_node_t));
    if (dir == NULL) return NULL;
    
    memset(dir, 0, sizeof(fs_node_t));
    strcpy(dir->name, name);
    dir->type = FS_DIRECTORY;
    dir->parent = current_dir;
    dir->child_count = 0;
    dir->created = get_time();
    dir->modified = dir->created;
    
    current_dir->children[current_dir->child_count++] = dir;
    current_dir->modified = get_time();
    
    return dir;
}

int fs_rmdir(const char* name) {
    fs_node_t* dir = find_child(current_dir, name);
    
    if (dir == NULL || dir->type != FS_DIRECTORY) {
        return -1;
    }
    
    if (dir->child_count > 0) {
        return -2;  // Directory not empty
    }
    
    // Remove from parent's children
    for (size_t i = 0; i < current_dir->child_count; i++) {
        if (current_dir->children[i] == dir) {
            // Shift remaining children
            for (size_t j = i; j < current_dir->child_count - 1; j++) {
                current_dir->children[j] = current_dir->children[j + 1];
            }
            current_dir->child_count--;
            break;
        }
    }
    
    kfree(dir);
    current_dir->modified = get_time();
    return 0;
}

int fs_chdir(const char* path) {
    fs_node_t* node = fs_resolve_path(path);
    
    if (node == NULL || node->type != FS_DIRECTORY) {
        return -1;
    }
    
    current_dir = node;
    return 0;
}

fs_node_t** fs_readdir(fs_node_t* dir, size_t* count) {
    if (dir == NULL || dir->type != FS_DIRECTORY) {
        *count = 0;
        return NULL;
    }
    
    *count = dir->child_count;
    return dir->children;
}

fs_node_t* fs_create(const char* name) {
    if (name == NULL || strlen(name) == 0 || strlen(name) >= FS_MAX_NAME) {
        return NULL;
    }
    
    if (strchr(name, '/') != NULL) {
        return NULL;
    }
    
    if (current_dir->child_count >= FS_MAX_CHILDREN) {
        return NULL;
    }
    
    if (find_child(current_dir, name) != NULL) {
        return NULL;  // Already exists
    }
    
    fs_node_t* file = (fs_node_t*)kmalloc(sizeof(fs_node_t));
    if (file == NULL) return NULL;
    
    memset(file, 0, sizeof(fs_node_t));
    strcpy(file->name, name);
    file->type = FS_FILE;
    file->parent = current_dir;
    file->size = 0;
    file->data = NULL;
    file->created = get_time();
    file->modified = file->created;
    
    current_dir->children[current_dir->child_count++] = file;
    current_dir->modified = get_time();
    
    return file;
}

int fs_delete(const char* name) {
    fs_node_t* file = find_child(current_dir, name);
    
    if (file == NULL) {
        return -1;
    }
    
    if (file->type == FS_DIRECTORY && file->child_count > 0) {
        return -2;  // Can't delete non-empty directory
    }
    
    // Remove from parent's children
    for (size_t i = 0; i < current_dir->child_count; i++) {
        if (current_dir->children[i] == file) {
            for (size_t j = i; j < current_dir->child_count - 1; j++) {
                current_dir->children[j] = current_dir->children[j + 1];
            }
            current_dir->child_count--;
            break;
        }
    }
    
    if (file->data != NULL) {
        kfree(file->data);
    }
    kfree(file);
    current_dir->modified = get_time();
    
    return 0;
}

int fs_write(fs_node_t* file, const void* data, size_t size) {
    if (file == NULL || file->type != FS_FILE) {
        return -1;
    }
    
    if (size > FS_MAX_FILE_SIZE) {
        return -2;
    }
    
    // Free old data
    if (file->data != NULL) {
        kfree(file->data);
    }
    
    if (size == 0) {
        file->data = NULL;
        file->size = 0;
    } else {
        file->data = (uint8_t*)kmalloc(size);
        if (file->data == NULL) {
            return -3;
        }
        memcpy(file->data, data, size);
        file->size = size;
    }
    
    file->modified = get_time();
    return 0;
}

int fs_append(fs_node_t* file, const void* data, size_t size) {
    if (file == NULL || file->type != FS_FILE) {
        return -1;
    }
    
    if (file->size + size > FS_MAX_FILE_SIZE) {
        return -2;
    }
    
    if (size == 0) {
        return 0;
    }
    
    uint8_t* new_data = (uint8_t*)kmalloc(file->size + size);
    if (new_data == NULL) {
        return -3;
    }
    
    if (file->data != NULL) {
        memcpy(new_data, file->data, file->size);
        kfree(file->data);
    }
    
    memcpy(new_data + file->size, data, size);
    file->data = new_data;
    file->size += size;
    file->modified = get_time();
    
    return 0;
}

int fs_read(fs_node_t* file, void* buffer, size_t size, size_t offset) {
    if (file == NULL || file->type != FS_FILE || buffer == NULL) {
        return -1;
    }
    
    if (offset >= file->size) {
        return 0;  // Nothing to read
    }
    
    size_t bytes_to_read = size;
    if (offset + size > file->size) {
        bytes_to_read = file->size - offset;
    }
    
    memcpy(buffer, file->data + offset, bytes_to_read);
    return bytes_to_read;
}

fs_node_t* fs_find(const char* name) {
    return find_child(current_dir, name);
}

int fs_rename(const char* old_name, const char* new_name) {
    if (new_name == NULL || strlen(new_name) == 0 || strlen(new_name) >= FS_MAX_NAME) {
        return -1;
    }
    
    fs_node_t* node = find_child(current_dir, old_name);
    if (node == NULL) {
        return -2;
    }
    
    if (find_child(current_dir, new_name) != NULL) {
        return -3;  // New name already exists
    }
    
    strcpy(node->name, new_name);
    node->modified = get_time();
    
    return 0;
}

int fs_copy(const char* src, const char* dest) {
    fs_node_t* src_file = find_child(current_dir, src);
    if (src_file == NULL || src_file->type != FS_FILE) {
        return -1;
    }
    
    fs_node_t* dest_file = fs_create(dest);
    if (dest_file == NULL) {
        return -2;
    }
    
    if (src_file->size > 0) {
        return fs_write(dest_file, src_file->data, src_file->size);
    }
    
    return 0;
}

// ============================================================================
// Disk Persistence - Simple flat file format
// ============================================================================

#define FS_MAGIC        0x4B414946  // "KAIF" - KaiOS File System
#define FS_VERSION      1
#define FS_START_SECTOR 100         // Start saving at sector 100

// On-disk file entry structure (fixed size for easy serialization)
typedef struct {
    char name[FS_MAX_NAME];
    uint8_t type;           // 0 = file, 1 = directory
    uint32_t size;          // File size
    uint32_t parent_id;     // Parent node ID
    uint32_t data_sector;   // Starting sector for file data
    uint32_t data_sectors;  // Number of sectors used for data
} PACKED disk_entry_t;

// On-disk header
typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t entry_count;
    uint32_t next_data_sector;
} PACKED disk_header_t;

static bool disk_available = false;

bool fs_has_disk(void) {
    return disk_available && ata_is_present();
}

// Assign IDs to nodes for serialization
static uint32_t assign_node_ids(fs_node_t* node, uint32_t id, fs_node_t** node_list, uint32_t max_nodes) {
    if (node == NULL || id >= max_nodes) return id;
    
    node_list[id] = node;
    uint32_t next_id = id + 1;
    
    for (size_t i = 0; i < node->child_count; i++) {
        next_id = assign_node_ids(node->children[i], next_id, node_list, max_nodes);
    }
    
    return next_id;
}

// Find node ID in list
static uint32_t find_node_id(fs_node_t* node, fs_node_t** node_list, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        if (node_list[i] == node) return i;
    }
    return 0;  // Default to root
}

bool fs_save(void) {
    if (!ata_is_present()) {
        return false;
    }
    
    // Collect all nodes
    fs_node_t* node_list[FS_MAX_FILES];
    memset(node_list, 0, sizeof(node_list));
    
    uint32_t node_count = assign_node_ids(root_dir, 0, node_list, FS_MAX_FILES);
    
    // Prepare header
    disk_header_t header;
    header.magic = FS_MAGIC;
    header.version = FS_VERSION;
    header.entry_count = node_count;
    header.next_data_sector = FS_START_SECTOR + 1 + ((node_count * sizeof(disk_entry_t) + 511) / 512);
    
    // Write header
    uint8_t sector_buf[512];
    memset(sector_buf, 0, 512);
    memcpy(sector_buf, &header, sizeof(header));
    
    if (!ata_write_sectors(FS_START_SECTOR, 1, sector_buf)) {
        return false;
    }
    
    // Write entries and file data
    uint32_t current_sector = FS_START_SECTOR + 1;
    uint32_t data_sector = header.next_data_sector;
    
    for (uint32_t i = 0; i < node_count; i++) {
        fs_node_t* node = node_list[i];
        
        disk_entry_t entry;
        memset(&entry, 0, sizeof(entry));
        strncpy(entry.name, node->name, FS_MAX_NAME - 1);
        entry.type = (uint8_t)node->type;
        entry.size = node->size;
        entry.parent_id = find_node_id(node->parent, node_list, node_count);
        
        // Write file data if it's a file with content
        if (node->type == FS_FILE && node->size > 0 && node->data != NULL) {
            entry.data_sector = data_sector;
            entry.data_sectors = (node->size + 511) / 512;
            
            // Write file data sectors
            uint8_t* data_buf = (uint8_t*)kmalloc(entry.data_sectors * 512);
            if (data_buf) {
                memset(data_buf, 0, entry.data_sectors * 512);
                memcpy(data_buf, node->data, node->size);
                ata_write_sectors(data_sector, entry.data_sectors, data_buf);
                kfree(data_buf);
            }
            
            data_sector += entry.data_sectors;
        } else {
            entry.data_sector = 0;
            entry.data_sectors = 0;
        }
        
        // Write entry (multiple entries per sector)
        uint32_t entries_per_sector = 512 / sizeof(disk_entry_t);
        uint32_t entry_sector = FS_START_SECTOR + 1 + (i / entries_per_sector);
        uint32_t entry_offset = (i % entries_per_sector) * sizeof(disk_entry_t);
        
        // Read sector, modify, write back
        if (entry_offset == 0) {
            memset(sector_buf, 0, 512);
        } else {
            ata_read_sectors(entry_sector, 1, sector_buf);
        }
        
        memcpy(sector_buf + entry_offset, &entry, sizeof(entry));
        
        // Write when sector is full or last entry
        if (entry_offset + sizeof(disk_entry_t) >= 512 || i == node_count - 1) {
            ata_write_sectors(entry_sector, 1, sector_buf);
        }
    }
    
    return true;
}

bool fs_load(void) {
    if (!ata_is_present()) {
        disk_available = false;
        return false;
    }
    
    disk_available = true;
    
    // Read header
    uint8_t sector_buf[512];
    if (!ata_read_sectors(FS_START_SECTOR, 1, sector_buf)) {
        return false;
    }
    
    disk_header_t* header = (disk_header_t*)sector_buf;
    
    // Check magic and version
    if (header->magic != FS_MAGIC || header->version != FS_VERSION) {
        return false;  // No valid filesystem, will use defaults
    }
    
    if (header->entry_count == 0 || header->entry_count > FS_MAX_FILES) {
        return false;
    }
    
    // Read all entries first
    disk_entry_t* entries = (disk_entry_t*)kmalloc(header->entry_count * sizeof(disk_entry_t));
    if (entries == NULL) {
        return false;
    }
    
    uint32_t entries_per_sector = 512 / sizeof(disk_entry_t);
    uint32_t sectors_needed = (header->entry_count + entries_per_sector - 1) / entries_per_sector;
    
    for (uint32_t s = 0; s < sectors_needed; s++) {
        ata_read_sectors(FS_START_SECTOR + 1 + s, 1, sector_buf);
        
        uint32_t start_entry = s * entries_per_sector;
        uint32_t end_entry = start_entry + entries_per_sector;
        if (end_entry > header->entry_count) end_entry = header->entry_count;
        
        for (uint32_t i = start_entry; i < end_entry; i++) {
            memcpy(&entries[i], sector_buf + ((i - start_entry) * sizeof(disk_entry_t)), sizeof(disk_entry_t));
        }
    }
    
    // Create nodes array
    fs_node_t** nodes = (fs_node_t**)kmalloc(header->entry_count * sizeof(fs_node_t*));
    if (nodes == NULL) {
        kfree(entries);
        return false;
    }
    
    // First pass: create all nodes
    for (uint32_t i = 0; i < header->entry_count; i++) {
        fs_node_t* node = (fs_node_t*)kmalloc(sizeof(fs_node_t));
        if (node == NULL) {
            // Cleanup on failure
            for (uint32_t j = 0; j < i; j++) {
                if (nodes[j]->data) kfree(nodes[j]->data);
                kfree(nodes[j]);
            }
            kfree(nodes);
            kfree(entries);
            return false;
        }
        
        memset(node, 0, sizeof(fs_node_t));
        strcpy(node->name, entries[i].name);
        node->type = (fs_type_t)entries[i].type;
        node->size = entries[i].size;
        node->created = get_time();
        node->modified = node->created;
        
        // Load file data
        if (node->type == FS_FILE && entries[i].size > 0 && entries[i].data_sectors > 0) {
            uint8_t* data_buf = (uint8_t*)kmalloc(entries[i].data_sectors * 512);
            if (data_buf) {
                ata_read_sectors(entries[i].data_sector, entries[i].data_sectors, data_buf);
                node->data = (uint8_t*)kmalloc(entries[i].size);
                if (node->data) {
                    memcpy(node->data, data_buf, entries[i].size);
                }
                kfree(data_buf);
            }
        }
        
        nodes[i] = node;
    }
    
    // Second pass: link parents and children
    for (uint32_t i = 0; i < header->entry_count; i++) {
        fs_node_t* node = nodes[i];
        uint32_t parent_id = entries[i].parent_id;
        
        if (i == 0) {
            // Root node
            node->parent = node;
        } else if (parent_id < header->entry_count) {
            node->parent = nodes[parent_id];
            
            // Add to parent's children
            fs_node_t* parent = nodes[parent_id];
            if (parent->child_count < FS_MAX_CHILDREN) {
                parent->children[parent->child_count++] = node;
            }
        }
    }
    
    // Replace root_dir
    if (root_dir != NULL) {
        // Free old tree (simplified - just free root for now)
        kfree(root_dir);
    }
    
    root_dir = nodes[0];
    current_dir = root_dir;
    
    kfree(nodes);
    kfree(entries);
    
    return true;
}
