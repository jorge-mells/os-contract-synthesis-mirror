#include "storage-driver.h"
#include "loader.h"

uint8_t toc_buffer[512];
uint64_t num_progs = 0;
void init_toc() {
    disk_read(0, toc_buffer);
    
    // Cache the number of programs so we don't have to keep rereading
    num_progs = *(uint64_t*)toc_buffer;
}

uint64_t get_num_progs() {
    return num_progs;
}

// assumptions: each program is fixed size(4kb hence 8 sectors) sector 0 is toc
void load_program(const uint8_t start_sector, uint64_t load_address) {
    for (int i = 0; i < 8; i++) {
        disk_read(start_sector + i, (void *) (load_address + (i * 512)));
    }
}