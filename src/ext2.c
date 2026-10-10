
#include "io.h"
#include "video.h"
#include "drivers/ata.h"
#include "ext2.h"

#define EXT2_MAGIC 0xEF53
#define EXT2_ROOT_INODE 2

static uint8_t sector[512];
static uint8_t block[1024];

static uint16_t read_u16(const uint8_t *p)
{
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static uint32_t read_u32(const uint8_t *p)
{
    return (uint32_t)p[0]
         | ((uint32_t)p[1] << 8)
         | ((uint32_t)p[2] << 16)
         | ((uint32_t)p[3] << 24);
}

static int read_block(uint32_t block_number)
{
    for (int i = 0; i < 2; i++)
    {
        if (ata_read_sector(block_number * 2 + i,
                            &block[i * 512]) != 0)
            return -1;
    }

    return 0;
}

void ext2_list_root(void)
{
    print("EXT2: Reading root directory...\n");

    /* EXT2 superblock starts at byte 1024, sector 2. */
    if (ata_read_sector(2, sector) != 0)
    {
        print("EXT2: Disk read failed.\n");
        return;
    }

    if (ata_read_sector(3, &block[512]) != 0)
    {
        print("EXT2: Disk read failed.\n");
        return;
    }

    /* Superblock is 1024 bytes; combine the two sectors. */
    for (int i = 0; i < 512; i++)
        block[i] = sector[i];

    if (read_u16(&block[56]) != EXT2_MAGIC)
    {
        print("EXT2: Invalid filesystem.\n");
        return;
    }

    uint32_t log_block_size = read_u32(&block[24]);

    if (log_block_size != 0)
    {
        print("EXT2: Only 1 KiB blocks are supported.\n");
        return;
    }

    uint32_t inodes_per_group = read_u32(&block[40]);
    uint32_t inode_size = read_u32(&block[76]) == 0
                        ? 128 : read_u16(&block[88]);

    if (inodes_per_group == 0 || inode_size != 128)
    {
        print("EXT2: Unsupported inode layout.\n");
        return;
    }

    /* For 1 KiB EXT2, the group descriptor table is block 2. */
    if (read_block(2) != 0)
    {
        print("EXT2: Disk read failed.\n");
        return;
    }

    uint32_t inode_table = read_u32(&block[8]);

    if (inode_table == 0 || read_block(inode_table) != 0)
    {
        print("EXT2: Cannot read inode table.\n");
        return;
    }

    /* Root inode is inode 2, at byte offset 128 in the table. */
    uint8_t root[128];

    for (int i = 0; i < 128; i++)
        root[i] = block[128 + i];

    uint16_t mode = read_u16(root);

    if ((mode & 0xF000) != 0x4000)
    {
        print("EXT2: Root inode is not a directory.\n");
        return;
    }

    uint32_t directory_size = read_u32(&root[4]);

    print("Files in /:\n");

    /* First 12 EXT2 inode block pointers are direct pointers. */
    uint32_t remaining = directory_size;

    for (int i = 0; i < 12 && remaining > 0; i++)
    {
        uint32_t data_block = read_u32(&root[40 + i * 4]);

        if (data_block == 0)
        {
            print("EXT2: Sparse directory block unsupported.\n");
            return;
        }

        if (read_block(data_block) != 0)
        {
            print("EXT2: Directory read failed.\n");
            return;
        }

        uint32_t limit = remaining < 1024 ? remaining : 1024;
        uint32_t offset = 0;

        while (offset + 8 <= limit)
        {
            uint32_t inode = read_u32(&block[offset]);
            uint16_t rec_len = read_u16(&block[offset + 4]);
            uint8_t name_len = block[offset + 6];

            if (rec_len < 8 || rec_len > limit - offset ||
                name_len > rec_len - 8)
            {
                print("EXT2: Invalid directory entry.\n");
                return;
            }

            if (inode != 0)
            {
                for (uint8_t j = 0; j < name_len; j++)
                    put_char((char)block[offset + 8 + j]);

                put_char('\n');
            }

            offset += rec_len;
        }

        remaining -= limit;
    }
}
