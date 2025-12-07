#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef struct {
    uint32_t nfiles;
} initrd_header_t;

typedef struct {
    uint8_t magic;
    char name[64];
    uint32_t offset;
    uint32_t length;
} initrd_file_header_t;

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s <output> <file1> <file2> ...\n", argv[0]);
        return 1;
    }
    
    int nfiles = argc - 2;
    FILE* output = fopen(argv[1], "wb");
    
    if (!output) {
        printf("Error: Cannot create output file\n");
        return 1;
    }
    
    // Write header
    initrd_header_t header;
    header.nfiles = nfiles;
    fwrite(&header, sizeof(initrd_header_t), 1, output);
    
    // Calculate offsets
    uint32_t offset = sizeof(initrd_header_t) + nfiles * sizeof(initrd_file_header_t);
    
    // Write file headers
    for (int i = 0; i < nfiles; i++) {
        FILE* f = fopen(argv[i + 2], "rb");
        if (!f) {
            printf("Error: Cannot open file %s\n", argv[i + 2]);
            fclose(output);
            return 1;
        }
        
        fseek(f, 0, SEEK_END);
        uint32_t length = ftell(f);
        fclose(f);
        
        initrd_file_header_t fheader;
        fheader.magic = 0xBF;
        strncpy(fheader.name, argv[i + 2], 63);
        fheader.name[63] = '\0';
        fheader.offset = offset;
        fheader.length = length;
        
        fwrite(&fheader, sizeof(initrd_file_header_t), 1, output);
        
        offset += length;
    }
    
    // Write file data
    for (int i = 0; i < nfiles; i++) {
        FILE* f = fopen(argv[i + 2], "rb");
        if (!f) continue;
        
        fseek(f, 0, SEEK_END);
        uint32_t length = ftell(f);
        fseek(f, 0, SEEK_SET);
        
        uint8_t* buffer = malloc(length);
        fread(buffer, 1, length, f);
        fwrite(buffer, 1, length, output);
        
        free(buffer);
        fclose(f);
    }
    
    fclose(output);
    printf("InitRD created: %s (%d files)\n", argv[1], nfiles);
    
    return 0;
}