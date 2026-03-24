/* fs/fat32.h - FAT32 USB Persistence Driver */

#ifndef FAT32_H
#define FAT32_H

#include "../kernel/include/kernel.h"

void fat32_init(void);
int  fat32_is_ready(void);
int  fat32_read_file(const char* filename, uint8_t* buffer, uint32_t max_size);
int  fat32_write_file(const char* filename, const uint8_t* buffer, uint32_t size);

#endif
