/**
 * @file storage-driver.h
 * @brief functions to communicate with permanent storage
 * @note if you want to change the driver, you would also have to fix the makefile, to point to your new driver file.
 */
#ifndef STORAGE_DRIVER_H
#define STORAGE_DRIVER_H
#include <stdint.h>

/**
 * @brief initialise disk communication. may differ depending on disk protocol
 */
void init_disk();

/**
 * @brief read disk sector @p sector_number into buffer pointing to @p buffer
 * 
 * @param sector_number The sector number to read. Arbitrary reads currently not supported.
 * @param buffer A pointer to the buffer to read into, which should be a valid physical ram address.
 */
void disk_read(uint64_t sector_number, void *buffer);
#endif