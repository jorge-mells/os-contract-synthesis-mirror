#ifndef SYSCALLS_H
#define SYSCALLS_H
#include <stdint.h>
#include <stddef.h>
uint64_t write(uint64_t fd, const char* buff, size_t count);
void sys_exit(uint64_t status);
#endif