/**
 * @file syscon.h
 * @brief poweroff, reboot, error handlers
 */
#ifndef SYSCON_H
#define SYSCON_H

#include "../platform/platform.h"

void poweroff(void);
void reboot(void);
void cleanup_and_exit(void);

#endif
