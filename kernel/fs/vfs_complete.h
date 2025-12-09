// kernel/fs/vfs_complete.h
#ifndef VFS_COMPLETE_H
#define VFS_COMPLETE_H

#include "../../include/types.h"

#define MAX_FILENAME 128
#define MAX_PATH 256
#define MAX_OPEN_FILES 256

// File types
#define FS_FILE        0x01
#define FS_DIRECTORY   0x02
#define FS_CHARDEVICE  0x03
#define FS_BLOCKDEVICE 0x04
#define FS_PIPE        0x05
#define FS_SYMLINK     0x06
#define FS_MOUNTPOINT  0x08

// File permissions
#define FS_READ   0x01
#define FS_WRITE  0x02
#define FS_EXEC   0x04

// Seek modes
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

// Forward declarations
typedef struct fs_node fs_node_t;
typedef struct dirent dirent_t;

// Directory entry
struct dirent {
    char name[MAX_FILENAME];
    uint32_t inode;
    uint8_t type;
};

// File operations
typedef uint32_t (*read_func_t)(fs_node_t*, uint32_t offset, uint32_t size, uint8_t* buffer);
typedef uint32_t (*write_func_t)(fs_node_t*, uint32_t offset, uint32_t size, uint8_t* buffer);
typedef void (*open_func_t)(fs_node_t*);
typedef void (*close_func_t)(fs_node_t*);
typedef dirent_t* (*readdir_func_t)(fs_node_t*, uint32_t index);
typedef fs_node_t* (*finddir_func_t)(fs_node_t*, const char* name);
typedef int (*create_func_t)(fs_node_t*, const char* name, uint32_t flags);
typedef int (*mkdir_func_t)(fs_node_t*, const char* name, uint32_t mode);
typedef int (*unlink_func_t)(fs_node_t*, const char* name);

// File system node
struct fs_node {
    char name[MAX_FILENAME];
    uint32_t inode;
    uint32_t length;
    uint32_t flags;
    uint32_t uid;
    uint32_t gid;
    uint32_t mask;
    
    // Function pointers
    read_func_t read;
    write_func_t write;
    open_func_t open;
    close_func_t close;
    readdir_func_t readdir;
    finddir_func_t finddir;
    create_func_t create;
    mkdir_func_t mkdir;
    unlink_func_t unlink;
    
    // Implementation-specific data
    void* impl;
};

// File descriptor
typedef struct {
    fs_node_t* node;
    uint32_t offset;
    uint32_t flags;
    int in_use;
} file_descriptor_t;

// VFS functions
void vfs_init(void);

// File operations
int vfs_open(const char* path, uint32_t flags);
void vfs_close(int fd);
int vfs_read(int fd, void* buffer, uint32_t size);
int vfs_write(int fd, const void* buffer, uint32_t size);
int vfs_seek(int fd, int offset, int whence);
int vfs_stat(const char* path, fs_node_t* stat_buf);

// Directory operations
int vfs_mkdir(const char* path, uint32_t mode);
int vfs_readdir(int fd, dirent_t* entry, uint32_t index);
fs_node_t* vfs_finddir(fs_node_t* node, const char* name);

// File management
int vfs_create(const char* path, uint32_t flags);
int vfs_unlink(const char* path);

// Mount operations
int vfs_mount(const char* path, fs_node_t* node);
int vfs_unmount(const char* path);

// Helper functions
fs_node_t* vfs_get_root(void);
void vfs_set_root(fs_node_t* root);
fs_node_t* vfs_resolve_path(const char* path);

#endif // VFS_COMPLETE_H