// kernel/fs/vfs_complete.c
#include "vfs_complete.h"
#include "../core/monitor.h"
#include "../core/panic.h"
#include "../mm/heap.h"
#include "../../lib/libc/string.h"

static fs_node_t* vfs_root = NULL;
static file_descriptor_t fd_table[MAX_OPEN_FILES];

// String helpers (assuming you have these)
extern int strcmp(const char* s1, const char* s2);
extern char* strcpy(char* dest, const char* src);
extern uint32_t strlen(const char* str);

void vfs_init(void) {
    print_string("  [VFS] Initializing Virtual File System...\n");
    
    // Clear file descriptor table
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        fd_table[i].in_use = 0;
        fd_table[i].node = NULL;
        fd_table[i].offset = 0;
        fd_table[i].flags = 0;
    }
    
    print_string("  [VFS] Ready\n");
}

void vfs_set_root(fs_node_t* root) {
    vfs_root = root;
    print_string("  [VFS] Root filesystem mounted\n");
}

fs_node_t* vfs_get_root(void) {
    return vfs_root;
}

// Allocate file descriptor
static int alloc_fd(void) {
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (!fd_table[i].in_use) {
            fd_table[i].in_use = 1;
            return i;
        }
    }
    return -1;  // No free descriptors
}

// Resolve path to fs_node
fs_node_t* vfs_resolve_path(const char* path) {
    if (!vfs_root) {
        return NULL;
    }
    
    // Handle root
    if (strcmp(path, "/") == 0) {
        return vfs_root;
    }
    
    // Start from root
    fs_node_t* current = vfs_root;
    
    // Skip leading slash
    if (path[0] == '/') {
        path++;
    }
    
    char component[MAX_FILENAME];
    uint32_t i = 0;
    
    while (*path) {
        // Extract path component
        i = 0;
        while (*path && *path != '/' && i < MAX_FILENAME - 1) {
            component[i++] = *path++;
        }
        component[i] = '\0';
        
        if (*path == '/') path++;
        
        // Find component in current directory
        if (current->finddir) {
            current = current->finddir(current, component);
            if (!current) {
                return NULL;  // Not found
            }
        } else {
            return NULL;  // Not a directory
        }
    }
    
    return current;
}

// Open file
int vfs_open(const char* path, uint32_t flags) {
    fs_node_t* node = vfs_resolve_path(path);
    if (!node) {
        return -1;  // File not found
    }
    
    int fd = alloc_fd();
    if (fd < 0) {
        return -1;  // No free descriptors
    }
    
    fd_table[fd].node = node;
    fd_table[fd].offset = 0;
    fd_table[fd].flags = flags;
    
    if (node->open) {
        node->open(node);
    }
    
    return fd;
}

// Close file
void vfs_close(int fd) {
    if (fd < 0 || fd >= MAX_OPEN_FILES || !fd_table[fd].in_use) {
        return;
    }
    
    fs_node_t* node = fd_table[fd].node;
    if (node && node->close) {
        node->close(node);
    }
    
    fd_table[fd].in_use = 0;
    fd_table[fd].node = NULL;
}

// Read from file
int vfs_read(int fd, void* buffer, uint32_t size) {
    if (fd < 0 || fd >= MAX_OPEN_FILES || !fd_table[fd].in_use) {
        return -1;
    }
    
    fs_node_t* node = fd_table[fd].node;
    if (!node || !node->read) {
        return -1;
    }
    
    uint32_t bytes_read = node->read(node, fd_table[fd].offset, size, (uint8_t*)buffer);
    fd_table[fd].offset += bytes_read;
    
    return bytes_read;
}

// Write to file
int vfs_write(int fd, const void* buffer, uint32_t size) {
    if (fd < 0 || fd >= MAX_OPEN_FILES || !fd_table[fd].in_use) {
        return -1;
    }
    
    fs_node_t* node = fd_table[fd].node;
    if (!node || !node->write) {
        return -1;
    }
    
    uint32_t bytes_written = node->write(node, fd_table[fd].offset, size, (uint8_t*)buffer);
    fd_table[fd].offset += bytes_written;
    
    return bytes_written;
}

// Seek in file
int vfs_seek(int fd, int offset, int whence) {
    if (fd < 0 || fd >= MAX_OPEN_FILES || !fd_table[fd].in_use) {
        return -1;
    }
    
    fs_node_t* node = fd_table[fd].node;
    if (!node) {
        return -1;
    }
    
    uint32_t new_offset;
    
    switch (whence) {
        case SEEK_SET:
            new_offset = offset;
            break;
        case SEEK_CUR:
            new_offset = fd_table[fd].offset + offset;
            break;
        case SEEK_END:
            new_offset = node->length + offset;
            break;
        default:
            return -1;
    }
    
    // Bounds check
    if (new_offset > node->length) {
        new_offset = node->length;
    }
    
    fd_table[fd].offset = new_offset;
    return new_offset;
}

// Create file
int vfs_create(const char* path, uint32_t flags) {
    // Extract directory and filename
    // Find last '/'
    const char* last_slash = NULL;
    for (const char* p = path; *p; p++) {
        if (*p == '/') {
            last_slash = p;
        }
    }
    
    if (!last_slash) {
        // No directory specified, use root
        if (vfs_root && vfs_root->create) {
            return vfs_root->create(vfs_root, path, flags);
        }
        return -1;
    }
    
    // Extract directory path
    char dir_path[MAX_PATH];
    uint32_t dir_len = last_slash - path;
    if (dir_len >= MAX_PATH) dir_len = MAX_PATH - 1;
    
    for (uint32_t i = 0; i < dir_len; i++) {
        dir_path[i] = path[i];
    }
    dir_path[dir_len] = '\0';
    
    // Get directory node
    fs_node_t* dir = vfs_resolve_path(dir_path);
    if (!dir || !dir->create) {
        return -1;
    }
    
    // Create file in directory
    return dir->create(dir, last_slash + 1, flags);
}

// Make directory
int vfs_mkdir(const char* path, uint32_t mode) {
    // Similar to create
    const char* last_slash = NULL;
    for (const char* p = path; *p; p++) {
        if (*p == '/') {
            last_slash = p;
        }
    }
    
    if (!last_slash) {
        if (vfs_root && vfs_root->mkdir) {
            return vfs_root->mkdir(vfs_root, path, mode);
        }
        return -1;
    }
    
    char dir_path[MAX_PATH];
    uint32_t dir_len = last_slash - path;
    if (dir_len >= MAX_PATH) dir_len = MAX_PATH - 1;
    
    for (uint32_t i = 0; i < dir_len; i++) {
        dir_path[i] = path[i];
    }
    dir_path[dir_len] = '\0';
    
    fs_node_t* dir = vfs_resolve_path(dir_path);
    if (!dir || !dir->mkdir) {
        return -1;
    }
    
    return dir->mkdir(dir, last_slash + 1, mode);
}

// Read directory entry
int vfs_readdir(int fd, dirent_t* entry, uint32_t index) {
    if (fd < 0 || fd >= MAX_OPEN_FILES || !fd_table[fd].in_use) {
        return -1;
    }
    
    fs_node_t* node = fd_table[fd].node;
    if (!node || !node->readdir) {
        return -1;
    }
    
    dirent_t* dir_entry = node->readdir(node, index);
    if (!dir_entry) {
        return -1;
    }
    
    // Copy entry
    strcpy(entry->name, dir_entry->name);
    entry->inode = dir_entry->inode;
    entry->type = dir_entry->type;
    
    return 0;
}

// Find in directory
fs_node_t* vfs_finddir(fs_node_t* node, const char* name) {
    if (!node || !node->finddir) {
        return NULL;
    }
    
    return node->finddir(node, name);
}

// Get file stats
int vfs_stat(const char* path, fs_node_t* stat_buf) {
    fs_node_t* node = vfs_resolve_path(path);
    if (!node) {
        return -1;
    }
    
    // Copy node info to stat buffer
    strcpy(stat_buf->name, node->name);
    stat_buf->inode = node->inode;
    stat_buf->length = node->length;
    stat_buf->flags = node->flags;
    stat_buf->uid = node->uid;
    stat_buf->gid = node->gid;
    stat_buf->mask = node->mask;
    
    return 0;
}

// Delete file
int vfs_unlink(const char* path) {
    const char* last_slash = NULL;
    for (const char* p = path; *p; p++) {
        if (*p == '/') {
            last_slash = p;
        }
    }
    
    if (!last_slash) {
        if (vfs_root && vfs_root->unlink) {
            return vfs_root->unlink(vfs_root, path);
        }
        return -1;
    }
    
    char dir_path[MAX_PATH];
    uint32_t dir_len = last_slash - path;
    if (dir_len >= MAX_PATH) dir_len = MAX_PATH - 1;
    
    for (uint32_t i = 0; i < dir_len; i++) {
        dir_path[i] = path[i];
    }
    dir_path[dir_len] = '\0';
    
    fs_node_t* dir = vfs_resolve_path(dir_path);
    if (!dir || !dir->unlink) {
        return -1;
    }
    
    return dir->unlink(dir, last_slash + 1);
}