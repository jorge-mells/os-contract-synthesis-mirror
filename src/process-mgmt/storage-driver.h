#ifndef STORAGE_DRIVER_H
#define STORAGE_DRIVER_H
#include <stdint.h>
void init_disk();
void disk_read(uint64_t sector_number, void *buffer);
#endif