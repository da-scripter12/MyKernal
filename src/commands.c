/* =========================
   COMMANDS
   ========================= */
void execute_command(void);
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
        /* nothing */
    }

    else if (string_equals(command, "help"))
    {
        print("Commands:\n");
        print("  help     - show commands\n");
        print("  clear    - clear screen\n");
        print("  echo     - print text\n");
        print("  about    - about this kernel\n");
        print("  reboot   - reboot computer\n");
    }

    else if (string_equals(command, "clear"))
    {
        clear_screen();
    }

    else if (string_equals(command, "about"))
    {
        print("MyOS - tiny x86 kernel\n");
        print("Written in C + Assembly.\n");
        print("Running directly on the machine.\n");
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

        uint8_t good = 0x02;

        while (good & 0x02)
        {
            good = inb(0x64);
        }

        outb(0x64, 0xFE);

        while (1)
        {
            __asm__ volatile ("hlt");
        }
    }
    else if (string_equals(command, "shutdown"))
    {
        print("Shutting down...\n");

        outb(0x604, 0x2000);

        while (1)
        {
            __asm__ volatile ("hlt");
        }
    }
    else if (string_equals(command, "version"))
    {
        print("GojiOS version 1.0\n");
    }
    else if (string_equals(command, "whoami"))
    {
        print("root\n");
    }
    else
    {
        print("Unknown command: ");
        print(command);
        print("\n");
    }

    command_length = 0;

    print("myOS> ");
}


