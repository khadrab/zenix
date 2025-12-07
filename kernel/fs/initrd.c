#include "initrd.h"
#include "../../lib/libc/string.h"
#include "../mm/heap.h"
#include "../core/monitor.h"

initrd_header_t* initrd_header;
initrd_file_header_t* file_headers;
fs_node_t* initrd_root;
fs_node_t* initrd_dev;
fs_node_t* root_nodes;
uint32_t nroot_nodes;

dirent_t dirent;

static uint32_t initrd_read(fs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer) {
    if (!node || !buffer) return 0;
    if (node->inode >= nroot_nodes) return 0;
    
    initrd_file_header_t header = file_headers[node->inode];
    if (offset > header.length) return 0;
    if (offset + size > header.length) size = header.length - offset;
    
    memcpy(buffer, (uint8_t*)(header.offset + offset), size);
    return size;
}

static dirent_t* initrd_readdir(fs_node_t* node, uint32_t index) {
    if (!node) return 0;
    
    if (node == initrd_root && index == 0) {
        strcpy(dirent.name, "dev");
        dirent.inode = 0;
        return &dirent;
    }
    
    if (index == 0) index = 1;
    if (index - 1 >= nroot_nodes) return 0;
    
    strcpy(dirent.name, root_nodes[index - 1].name);
    dirent.inode = root_nodes[index - 1].inode;
    return &dirent;
}

static fs_node_t* initrd_finddir(fs_node_t* node, char* name) {
    if (!node || !name) return 0;
    if (node == initrd_root && strcmp(name, "dev") == 0) return initrd_dev;
    
    for (uint32_t i = 0; i < nroot_nodes; i++) {
        if (strcmp(name, root_nodes[i].name) == 0) return &root_nodes[i];
    }
    return 0;
}

fs_node_t* initrd_init(uint32_t location) {
    if (location < 0x100000) return 0;
    
    // قراءة byte by byte - تجنب alignment issues
    uint8_t* data = (uint8_t*)location;
    
    print_string("       First 16 bytes: ");
    for(int i = 0; i < 16; i++) {
        uint8_t b = data[i];
        if (b < 0x10) print_char('0');
        print_hex(b);
        print_char(' ');
    }
    print_string("\n");
    
    // قراءة nfiles (little-endian, 4 bytes)
    nroot_nodes = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
    
    print_string("       nfiles = ");
    print_dec(nroot_nodes);
    print_string("\n");
    
    // تحقق معقول
    if (nroot_nodes == 0 || nroot_nodes > 10) {
        print_string("       [ERROR] Bad nfiles\n");
        return 0;
    }
    
    // تخطي header (4 bytes)
    uint32_t file_header_offset = 4;
    
    // إنشاء structures
    initrd_root = (fs_node_t*)kmalloc(sizeof(fs_node_t));
    if (!initrd_root) return 0;
    memset(initrd_root, 0, sizeof(fs_node_t));
    strcpy(initrd_root->name, "initrd");
    initrd_root->flags = FS_DIRECTORY;
    initrd_root->readdir = &initrd_readdir;
    initrd_root->finddir = &initrd_finddir;
    
    initrd_dev = (fs_node_t*)kmalloc(sizeof(fs_node_t));
    if (!initrd_dev) { kfree(initrd_root); return 0; }
    memset(initrd_dev, 0, sizeof(fs_node_t));
    strcpy(initrd_dev->name, "dev");
    initrd_dev->flags = FS_DIRECTORY;
    
    root_nodes = (fs_node_t*)kmalloc(sizeof(fs_node_t) * nroot_nodes);
    if (!root_nodes) { kfree(initrd_dev); kfree(initrd_root); return 0; }
    memset(root_nodes, 0, sizeof(fs_node_t) * nroot_nodes);
    
    // قراءة file headers يدوياً
    for (uint32_t i = 0; i < nroot_nodes; i++) {
        uint8_t* fh = data + file_header_offset + i * 73; // sizeof(initrd_file_header_t) = 73
        
        // magic (1 byte) + name (64 bytes) + offset (4) + length (4)
        // تخطي magic
        char* fname = (char*)(fh + 1);
        uint32_t foffset = fh[65] | (fh[66] << 8) | (fh[67] << 16) | (fh[68] << 24);
        uint32_t flength = fh[69] | (fh[70] << 8) | (fh[71] << 16) | (fh[72] << 24);
        
        strncpy(root_nodes[i].name, fname, 63);
        root_nodes[i].name[63] = '\0';
        root_nodes[i].flags = FS_FILE;
        root_nodes[i].read = &initrd_read;
        root_nodes[i].inode = i;
        root_nodes[i].length = flength;
        
        // Offset نسبة للـ location
        // في mkinitrd: offset = header + all_file_headers + previous_files
        // نحتاج نضيف location
        root_nodes[i].impl = location + foffset;
    }
    
    // تحديث file_headers pointer للقراءة
    file_headers = (initrd_file_header_t*)(location + 4);
    
    print_string("       [OK] Loaded ");
    print_dec(nroot_nodes);
    print_string(" files\n");
    
    return initrd_root;
}
