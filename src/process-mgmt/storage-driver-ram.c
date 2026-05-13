#include "storage-driver.h"
#include "../platform/platform.h"

void init_disk() {
    return;
}

void disk_read(uint64_t sector_number, void *buffer) {
    uint64_t *source = (uint64_t *)(DISK_START + (sector_number * 512));
    
    // Check if the destination is 8-byte aligned
    if (((uintptr_t)buffer & 0x7) == 0) {
        uint64_t *dst = (uint64_t *)buffer;
        // 512 bytes / 8 bytes per copy = 64 iterations
        for (int i = 0; i < 64; i++) {
            dst[i] = source[i];
        }
    } else {
        // Fallback: Byte-by-byte copy if alignment is weird (like 0x...4)
        uint8_t *src8 = (uint8_t *)source;
        uint8_t *dst8 = (uint8_t *)buffer;
        for (int i = 0; i < 512; i++) {
            dst8[i] = src8[i];
        }
    }
}