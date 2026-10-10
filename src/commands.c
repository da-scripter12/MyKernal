#include "io.h"
#include "video.h"
#include "commands.h"
#include "ext2.h"
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

char command[128];
int command_length = 0;

int string_equals(const char *a, const char *b)
{
    int i = 0;

    while (a[i] != '\0' && b[i] != '\0')
    {
        if (a[i] != b[i])
            return 0;

        i++;
    }

    return a[i] == '\0' && b[i] == '\0';
}

int starts_with(const char *text, const char *prefix)
{
    int i = 0;

    while (prefix[i] != '\0')
    {
        if (text[i] != prefix[i])
            return 0;

        i++;
    }

    return 1;
}

void execute_command(void)
{
    command[command_length] = '\0';
    put_char('\n');

    if (string_equals(command, ""))
    {
        /* Empty command. */
    }
    else if (string_equals(command, "help"))
    {
        print("Commands:\n");
        print("  help     - show commands\n");
        print("  clear    - clear screen\n");
        print("  echo     - print text\n");
        print("  about    - about this kernel\n");
        print("  reboot   - reboot computer\n");
        print("  shutdown - shut down computer\n");
        print("  version  - show version\n");
        print("  whoami   - show current user\n");
        print("  ls       - list files in root directory\n");
    }
    else if (string_equals(command, "clear"))
    {
        clear_screen();
    }
    else if (string_equals(command, "about"))
    {
        print("GojiOS - tiny x86 kernel\n");
        print("Written in C + Assembly.\n");
        print("Booted through GRUB.\n");
    }
    else if (starts_with(command, "echo "))
    {
        print(command + 5);
        put_char('\n');
    }
    else if (string_equals(command, "echo"))
    {
        put_char('\n');
    }
    else if (string_equals(command, "reboot"))
    {
        print("Rebooting...\n");

        /* Wait until the keyboard controller input buffer is empty. */
        while (inb(KEYBOARD_STATUS_PORT) & 0x02)
        {
        }

        outb(KEYBOARD_STATUS_PORT, 0xFE);

        for (;;)
            __asm__ volatile ("hlt");
    }
    else if (string_equals(command, "shutdown"))
    {
        print("Shutting down...\n");

        /*
         * QEMU/Bochs commonly support the ACPI shutdown port.
         * This is emulator-specific, not a universal PC shutdown method.
         */
        outw(0x604, 0x2000);

        for (;;)
            __asm__ volatile ("hlt");
    }
    else if (string_equals(command, "version"))
    {
        print("GojiOS version 1.0\n");
    }
    else if (string_equals(command, "whoami"))
    {
        print("root\n");
    }
    else if (string_equals(command, "ls"))
    {
        ext2_list_root();
    }
    else if (string_equals(command, "dskinf"  ))
    {
        ext2_disk_test();
    }
    else
    {
        print("Unknown command: ");
        print(command);
        put_char('\n');
    }

    command_length = 0;
    command[0] = '\0';

    print("Gsh> ");
}
