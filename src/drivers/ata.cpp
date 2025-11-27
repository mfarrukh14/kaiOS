/*
 * KaiOS - ATA/IDE Disk Driver
 * PIO mode disk access for reading/writing sectors
 */

#include "include/drivers/ata.h"
#include "include/drivers/io.h"
#include "include/kernel/string.h"

static bool ata_present = false;

// Wait for drive to be ready
static bool ata_wait_ready(void) {
    int timeout = 500000;  // Increased timeout for warm reboot
    while (timeout--) {
        uint8_t status = inb(ATA_PRIMARY_STATUS);
        if (!(status & ATA_STATUS_BSY)) {
            return true;
        }
        // Small busy-wait
        for (volatile int i = 0; i < 100; i++);
    }
    return false;
}

// Wait for data request
static bool ata_wait_drq(void) {
    int timeout = 500000;  // Increased timeout
    while (timeout--) {
        uint8_t status = inb(ATA_PRIMARY_STATUS);
        if (status & ATA_STATUS_ERR) {
            return false;
        }
        if (status & ATA_STATUS_DRQ) {
            return true;
        }
        // Small busy-wait
        for (volatile int i = 0; i < 100; i++);
    }
    return false;
}

// 400ns delay by reading status port
static void ata_delay(void) {
    for (int i = 0; i < 14; i++) {
        inb(ATA_PRIMARY_CONTROL);
    }
}

// Full hardware reset
static void ata_reset(void) {
    // Assert SRST (software reset)
    outb(ATA_PRIMARY_CONTROL, 0x04);
    
    // Wait at least 5 microseconds
    for (volatile int i = 0; i < 10000; i++);
    
    // Deassert SRST
    outb(ATA_PRIMARY_CONTROL, 0x00);
    
    // Wait at least 2 ms for drive to recover
    for (volatile int i = 0; i < 100000; i++);
    
    // Wait for BSY to clear
    ata_wait_ready();
}

void ata_init(void) {
    ata_present = false;
    
    // Full hardware reset for warm boot compatibility
    ata_reset();
    
    // Extra delay after reset
    for (volatile int i = 0; i < 50000; i++);
    
    // Check if drive exists
    ata_present = ata_identify();
    
    // If first attempt fails, try again with longer delay
    if (!ata_present) {
        ata_reset();
        for (volatile int i = 0; i < 100000; i++);
        ata_present = ata_identify();
    }
}

bool ata_identify(void) {
    // Select master drive
    outb(ATA_PRIMARY_DRIVE_HEAD, ATA_MASTER);
    ata_delay();
    
    // Clear sector count and LBA registers
    outb(ATA_PRIMARY_SECCOUNT, 0);
    outb(ATA_PRIMARY_LBA_LO, 0);
    outb(ATA_PRIMARY_LBA_MID, 0);
    outb(ATA_PRIMARY_LBA_HI, 0);
    
    // Send IDENTIFY command
    outb(ATA_PRIMARY_COMMAND, ATA_CMD_IDENTIFY);
    ata_delay();
    
    // Check if drive exists
    uint8_t status = inb(ATA_PRIMARY_STATUS);
    if (status == 0) {
        return false;  // No drive
    }
    
    // Wait for BSY to clear
    if (!ata_wait_ready()) {
        return false;
    }
    
    // Check for non-ATA drive
    if (inb(ATA_PRIMARY_LBA_MID) != 0 || inb(ATA_PRIMARY_LBA_HI) != 0) {
        return false;  // Not ATA
    }
    
    // Wait for DRQ or ERR
    int timeout = 100000;
    while (timeout--) {
        status = inb(ATA_PRIMARY_STATUS);
        if (status & ATA_STATUS_ERR) {
            return false;
        }
        if (status & ATA_STATUS_DRQ) {
            break;
        }
    }
    
    if (timeout <= 0) {
        return false;
    }
    
    // Read and discard identify data (256 words)
    for (int i = 0; i < 256; i++) {
        inw(ATA_PRIMARY_DATA);
    }
    
    return true;
}

bool ata_read_sectors(uint32_t lba, uint8_t sector_count, void* buffer) {
    if (!ata_present || sector_count == 0) {
        return false;
    }
    
    // Wait for drive ready
    if (!ata_wait_ready()) {
        return false;
    }
    
    // Select drive and set LBA mode + high 4 bits of LBA
    outb(ATA_PRIMARY_DRIVE_HEAD, ATA_MASTER | ((lba >> 24) & 0x0F));
    ata_delay();
    
    // Set sector count
    outb(ATA_PRIMARY_SECCOUNT, sector_count);
    
    // Set LBA address
    outb(ATA_PRIMARY_LBA_LO, lba & 0xFF);
    outb(ATA_PRIMARY_LBA_MID, (lba >> 8) & 0xFF);
    outb(ATA_PRIMARY_LBA_HI, (lba >> 16) & 0xFF);
    
    // Send read command
    outb(ATA_PRIMARY_COMMAND, ATA_CMD_READ_SECTORS);
    
    uint16_t* buf = (uint16_t*)buffer;
    
    for (int s = 0; s < sector_count; s++) {
        // Wait for data
        if (!ata_wait_drq()) {
            return false;
        }
        
        // Read 256 words (512 bytes)
        for (int i = 0; i < 256; i++) {
            buf[s * 256 + i] = inw(ATA_PRIMARY_DATA);
        }
        
        ata_delay();
    }
    
    return true;
}

bool ata_write_sectors(uint32_t lba, uint8_t sector_count, const void* buffer) {
    if (!ata_present || sector_count == 0) {
        return false;
    }
    
    // Wait for drive ready
    if (!ata_wait_ready()) {
        return false;
    }
    
    // Select drive and set LBA mode + high 4 bits of LBA
    outb(ATA_PRIMARY_DRIVE_HEAD, ATA_MASTER | ((lba >> 24) & 0x0F));
    ata_delay();
    
    // Set sector count
    outb(ATA_PRIMARY_SECCOUNT, sector_count);
    
    // Set LBA address
    outb(ATA_PRIMARY_LBA_LO, lba & 0xFF);
    outb(ATA_PRIMARY_LBA_MID, (lba >> 8) & 0xFF);
    outb(ATA_PRIMARY_LBA_HI, (lba >> 16) & 0xFF);
    
    // Send write command
    outb(ATA_PRIMARY_COMMAND, ATA_CMD_WRITE_SECTORS);
    
    const uint16_t* buf = (const uint16_t*)buffer;
    
    for (int s = 0; s < sector_count; s++) {
        // Wait for drive ready
        if (!ata_wait_drq()) {
            return false;
        }
        
        // Write 256 words (512 bytes)
        for (int i = 0; i < 256; i++) {
            outw(ATA_PRIMARY_DATA, buf[s * 256 + i]);
        }
        
        ata_delay();
    }
    
    // Flush cache
    outb(ATA_PRIMARY_COMMAND, ATA_CMD_FLUSH);
    ata_wait_ready();
    
    return true;
}

bool ata_is_present(void) {
    return ata_present;
}
