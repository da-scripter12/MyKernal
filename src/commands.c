#include "io.h"
#include "video.h"
#include "commands.h"
#include "ext2.h"

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
        print("  dskinf   - show EXT2 disk information\n");
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
