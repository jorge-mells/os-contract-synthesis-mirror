#ifndef KMALLOC_H
#define KMALLOC_H
/* Declare the symbols from the linker script */
#include <stdint.h>
extern char _heap_start[];
extern char _heap_end[];
extern char __stack_top[];
extern char GUARD_HOLE[];

void setup_kheap();
#endif