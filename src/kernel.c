#include "io.h"
#include "video.h"
#include "keyboard.h"
#include <stdint.h>
#include "drivers/ata.h"

static uint8_t sector[512];

void ext2_disk_test(void)
{
    print("EXT2: Reading superblock...\n");

    if (ata_read_sector(2, sector) != 0) {
        print("EXT2: Disk read failed.\n");
        return;
    }

    uint16_t magic =
        (uint16_t)sector[56] |
        ((uint16_t)sector[57] << 8);

    if (magic == 0xEF53) {
        print("EXT2: Superblock found! Magic = 0xEF53\n");
    } else {
        print("EXT2: Invalid magic number.\n");
    }
}

void kernel_main(void)
{
    clear_screen();

    print("========================================\n");
    print("              GojiOS Kernel\n");
    print("========================================\n\n");
    print("Welcome to GojiOS!\n");
    print("Type 'help' for commands.\n\n");
    ext2_disk_test();

    print("Gsh> ");

    for (;;)
    {
        if (inb(KEYBOARD_STATUS_PORT) & 1)
        {
            keyboard_handler();
        }

        __asm__ volatile ("pause");
    }
}
