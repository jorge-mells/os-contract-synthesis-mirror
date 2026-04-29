#ifndef LOADER_H
#define LOADER_H
#include <stdint.h>
// from the python script
struct toc_entry {
    char name[32];        // 32 bytes (ljust(32, b'\0'))
    uint64_t start_sector;// <Q (64-bit unsigned)
    uint64_t size_bytes;  // <Q (64-bit unsigned)
};
extern uint8_t toc_buffer[512];
extern uint64_t num_progs;
void init_toc();
uint64_t get_num_progs();
void load_program(const uint8_t start_sector, uint64_t load_address);
#endif