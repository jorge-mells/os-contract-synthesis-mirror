#include "storage-driver.h"
#include "loader.h"

uint8_t toc_buffer[512];
uint64_t total_prog_count = 0;
void load_prg_info() {
    total_prog_count = 2;
    return;
}

void load_prg_info_impl() {
    disk_read(0, toc_buffer);
    total_prog_count = *(uint64_t*)toc_buffer;
}


void load_program_impl(const uint64_t start_sector, uint64_t load_address) {
    for (int i = 0; i < 8; i++) {
        disk_read(start_sector + i, (void *)(uintptr_t) (load_address + (i * 512)));
    }
}


void load_program(const uint64_t start_sector, uint64_t load_address) {
    return;
}