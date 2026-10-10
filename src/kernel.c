#include "io.h"
#include "video.h"
#include "keyboard.h"
#include <stdint.h>
#include "drivers/ata.h"

static uint8_t sector[512];

static uint32_t ext2_read_u32(const uint8_t *data)
{
    return (uint32_t)data[0]
         | ((uint32_t)data[1] << 8)
         | ((uint32_t)data[2] << 16)
         | ((uint32_t)data[3] << 24);
}

static void ext2_print_u32(uint32_t value)
{
    char digits[11];
    int i = 0;

    if (value == 0) {
        print("0");
        return;
    }

    while (value > 0) {
        digits[i++] = '0' + (value % 10);
        value /= 10;
    }

    while (i > 0) {
        char digit[2] = { digits[--i], '\0' };
        print(digit);
    }
}

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

    if (magic != 0xEF53) {
        print("EXT2: Invalid magic number.\n");
        return;
    }

    uint32_t total_inodes = ext2_read_u32(&sector[0]);
    uint32_t total_blocks = ext2_read_u32(&sector[4]);
    uint32_t first_data_block = ext2_read_u32(&sector[20]);
    uint32_t log_block_size = ext2_read_u32(&sector[24]);
    uint32_t blocks_per_group = ext2_read_u32(&sector[32]);
    uint32_t inodes_per_group = ext2_read_u32(&sector[40]);

    if (log_block_size > 6) {
        print("EXT2: Unsupported block size.\n");
        return;
    }

    uint32_t block_size = 1024U << log_block_size;

    print("\n=== EXT2 SUPERBLOCK ===\n");

    print("Magic: 0xEF53 (valid)\n");

    print("Total inodes: ");
    ext2_print_u32(total_inodes);
    print("\n");

    print("Total blocks: ");
    ext2_print_u32(total_blocks);
    print("\n");

    print("Block size: ");
    ext2_print_u32(block_size);
    print(" bytes\n");

    print("First data block: ");
    ext2_print_u32(first_data_block);
    print("\n");

    print("Blocks per group: ");
    ext2_print_u32(blocks_per_group);
    print("\n");

    print("Inodes per group: ");
    ext2_print_u32(inodes_per_group);
    print("\n");

    print("EXT2: Superblock parsed successfully!\n");
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
