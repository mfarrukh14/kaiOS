/*
 * KaiOS - ATA/IDE Disk Driver Header
 * Provides disk read/write functionality using PIO mode
 */

#ifndef KAIOS_ATA_H
#define KAIOS_ATA_H

#include "include/kernel/types.h"

// ATA ports (Primary bus)
#define ATA_PRIMARY_DATA         0x1F0
#define ATA_PRIMARY_ERROR        0x1F1
#define ATA_PRIMARY_SECCOUNT     0x1F2
#define ATA_PRIMARY_LBA_LO       0x1F3
#define ATA_PRIMARY_LBA_MID      0x1F4
#define ATA_PRIMARY_LBA_HI       0x1F5
#define ATA_PRIMARY_DRIVE_HEAD   0x1F6
#define ATA_PRIMARY_STATUS       0x1F7
#define ATA_PRIMARY_COMMAND      0x1F7
#define ATA_PRIMARY_CONTROL      0x3F6

// ATA commands
#define ATA_CMD_READ_SECTORS     0x20
#define ATA_CMD_WRITE_SECTORS    0x30
#define ATA_CMD_IDENTIFY         0xEC
#define ATA_CMD_FLUSH            0xE7

// ATA status bits
#define ATA_STATUS_ERR           0x01
#define ATA_STATUS_DRQ           0x08
#define ATA_STATUS_SRV           0x10
#define ATA_STATUS_DF            0x20
#define ATA_STATUS_RDY           0x40
#define ATA_STATUS_BSY           0x80

// Sector size
#define ATA_SECTOR_SIZE          512

// Drive selection
#define ATA_MASTER               0xE0
#define ATA_SLAVE                0xF0

// ATA functions
void ata_init(void);
bool ata_identify(void);
bool ata_read_sectors(uint32_t lba, uint8_t sector_count, void* buffer);
bool ata_write_sectors(uint32_t lba, uint8_t sector_count, const void* buffer);
bool ata_is_present(void);

#endif // KAIOS_ATA_H
