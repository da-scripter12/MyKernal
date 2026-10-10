#include "ata.h"

#define ATA_DATA       0x1F0
#define ATA_ERROR      0x1F1
#define ATA_SECCOUNT   0x1F2
#define ATA_LBA_LOW    0x1F3
#define ATA_LBA_MID    0x1F4
#define ATA_LBA_HIGH   0x1F5
#define ATA_DRIVE      0x1F6
#define ATA_STATUS     0x1F7
#define ATA_COMMAND    0x1F7
#define ATA_ALT_STATUS 0x3F6

#define ATA_BSY  0x80
#define ATA_DRQ  0x08
#define ATA_DF   0x20
#define ATA_ERR  0x01

#define ATA_READ 0x20
#define ATA_LBA28_MAX 0x0FFFFFFF

static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile ("outb %0, %1"
                      :
                      : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t value;

    __asm__ volatile ("inb %1, %0"
                      : "=a"(value)
                      : "Nd"(port));

    return value;
}

static inline uint16_t inw(uint16_t port)
{
    uint16_t value;

    __asm__ volatile ("inw %1, %0"
                      : "=a"(value)
                      : "Nd"(port));

    return value;
}

static void ata_delay_400ns(void)
{
    for (int i = 0; i < 4; i++)
        (void)inb(ATA_ALT_STATUS);
}

/* Returns 0 on success and -1 on failure. */
static int ata_wait(uint8_t required_status)
{
    for (uint32_t timeout = 0; timeout < 1000000; timeout++) {
        uint8_t status = inb(ATA_STATUS);

        if (status == 0x00 || status == 0xFF)
            return -1;

        if (status & ATA_BSY)
            continue;

        if (status & (ATA_ERR | ATA_DF))
            return -1;

        if ((status & required_status) == required_status)
            return 0;
    }

    return -1;
}

int ata_read_sector(uint32_t lba, uint8_t *buffer)
{
    if (buffer == 0 || lba > ATA_LBA28_MAX)
        return -1;

    if (ata_wait(0) != 0)
        return -1;

    /* Select primary-master drive and the top LBA bits. */
    outb(ATA_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));
    ata_delay_400ns();

    if (ata_wait(0) != 0)
        return -1;

    outb(ATA_SECCOUNT, 1);
    outb(ATA_LBA_LOW,  (uint8_t)lba);
    outb(ATA_LBA_MID,  (uint8_t)(lba >> 8));
    outb(ATA_LBA_HIGH, (uint8_t)(lba >> 16));

    outb(ATA_COMMAND, ATA_READ);

    if (ata_wait(ATA_DRQ) != 0)
        return -1;

    /* One sector contains 256 16-bit words. */
    for (uint32_t i = 0; i < 256; i++) {
        uint16_t word = inw(ATA_DATA);

        buffer[i * 2]     = (uint8_t)word;
        buffer[i * 2 + 1] = (uint8_t)(word >> 8);
    }

    return 0;
}