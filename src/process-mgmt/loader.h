/**
 * @file loader.h
 * @brief Utility functions to read various types of data off the storage medium.
 * 
 * The disk attached must contain a Table of Contents (TOC) in which each 
 * entry is defined as given in @ref toc_entry. The TOC will be loaded 
 * into the memory buffer @ref toc_buffer.
 * 
 * @note This TOC must be placed in sector 0 of the disk immediately following 
 * the program count (i.e., toc_buffer[0...7] contains the total_prog_count).
 */
#ifndef LOADER_H
#define LOADER_H
#include <stdint.h>

/**
 * @brief memory map for TOC on disk.
 */
struct toc_entry {
    char name[32];        // 32 bytes (ljust(32, b'\0'))
    uint64_t start_sector;// <Q (64-bit unsigned)
    uint64_t size_bytes;  // <Q (64-bit unsigned)
};

extern uint8_t toc_buffer[512];
extern uint64_t total_prog_count;

/**
 * @brief Load TOC into the OS. This should be called only once.
 */
void load_prg_info();

/**
 * @brief Load a program from the disk into @ref load_address. 
 * 
 * @param start_sector The sector at which the program starts from
 * @param load_address The location in RAM where the program should be loaded
 * 
 * @warning Each program is assumed to be of fixed size and is stored in 8 sectors!
 * @see linker.ld to find out the size of each program
 */
void load_program(const uint8_t start_sector, uint64_t load_address);
#endif