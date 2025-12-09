#ifndef INITRD_H
#define INITRD_H

#include "vfs.h"

typedef struct {
    uint32_t nfiles;
} __attribute__((packed)) initrd_header_t;

typedef struct {
    uint8_t magic;     // 0xBF
    char name[64];
    uint32_t offset;
    uint32_t length;
} __attribute__((packed)) initrd_file_header_t;

fs_node_t* initrd_init(uint32_t location);

#endif
