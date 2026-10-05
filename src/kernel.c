#define KEYBOARD_DATA_PORT   0x60
#define KEYBOARD_STATUS_PORT 0x64
#define VIDEO_MEMORY          0xB8000

#define WIDTH  80
#define HEIGHT 25

typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;

static inline uint8_t inb(uint16_t port)
{
    uint8_t value;

    __asm__ volatile (
        "inb %1, %0"
        : "=a"(value)
        : "Nd"(port)
    );

    return value;
}

static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile (
        "outb %0, %1"
        :
        : "a"(value), "Nd"(port)
    );
}


/* =========================
   VIDEO
   ========================= */

volatile uint16_t *video = (volatile uint16_t *)VIDEO_MEMORY;

int cursor_x = 0;
int cursor_y = 0;

uint8_t color = 0x07;


void put_char(char c)
{
    if (c == '\n')
    {
        cursor_x = 0;
        cursor_y++;
    }
    else
    {
        video[cursor_y * WIDTH + cursor_x]
            = ((uint16_t)color << 8) | c;

        cursor_x++;

        if (cursor_x >= WIDTH)
        {
            cursor_x = 0;
            cursor_y++;
        }
    }

    if (cursor_y >= HEIGHT)
    {
        for (int y = 1; y < HEIGHT; y++)
        {
            for (int x = 0; x < WIDTH; x++)
            {
                video[(y - 1) * WIDTH + x] =
                    video[y * WIDTH + x];
            }
        }

        for (int x = 0; x < WIDTH; x++)
        {
            video[(HEIGHT - 1) * WIDTH + x] =
                ((uint16_t)color << 8) | ' ';
        }

        cursor_y = HEIGHT - 1;
    }
}


void print(const char *text)
{
    for (int i = 0; text[i] != '\0'; i++)
    {
        put_char(text[i]);
    }
}


void clear_screen(void)
{
    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            video[y * WIDTH + x] =
                ((uint16_t)color << 8) | ' ';
        }
    }

    cursor_x = 0;
    cursor_y = 0;
}


/* =========================
   KEYBOARD
   ========================= */

const char keyboard_map[128] =
{
    0,
    27,

    '1','2','3','4','5','6','7','8','9','0',
    '-','=',
    '\b',
    '\t',

    'q','w','e','r','t','y','u','i','o','p',
    '[',']',
    '\n',

    0,

    'a','s','d','f','g','h','j','k','l',
    ';','\'','`',

    0,
    '\\',

    'z','x','c','v','b','n','m',
    ',','.','/',

    0,
    '*',

    0,
    ' ',

    /* rest */
};


char command[128];
int command_length = 0;


/* =========================
   COMMANDS
   ========================= */

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

    else
    {
        print("Unknown command: ");
        print(command);
        print("\n");
    }

    command_length = 0;

    print("myOS> ");
}


/* =========================
   KEYBOARD HANDLING
   ========================= */

void keyboard_handler(void)
{
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);

    /*
       Ignore key-release codes.
       Key releases have the high bit set.
    */
    if (scancode & 0x80)
        return;

    if (scancode >= 128)
        return;

    char c = keyboard_map[scancode];

    if (c == 0)
        return;


    /* BACKSPACE */

    if (c == '\b')
    {
        if (command_length > 0)
        {
            command_length--;
            cursor_x--;

            if (cursor_x < 0)
            {
                cursor_x = WIDTH - 1;
                cursor_y--;
            }

            video[cursor_y * WIDTH + cursor_x] =
                ((uint16_t)color << 8) | ' ';
        }

        return;
    }


    /* ENTER */

    if (c == '\n')
    {
        execute_command();
        return;
    }


    /* NORMAL CHARACTER */

    if (command_length < 127)
    {
        command[command_length] = c;
        command_length++;

        put_char(c);
    }
}


/* =========================
   KERNEL ENTRY
   ========================= */

void kernel_main(void)
{
    clear_screen();

    print("========================================\n");
    print("              MyOS Kernel\n");
    print("========================================\n");
    print("\n");
    print("Welcome to MyOS!\n");
    print("Type 'help' for commands.\n");
    print("\n");

    print("myOS> ");

    while (1)
    {
        if (inb(KEYBOARD_STATUS_PORT) & 1)
        {
            keyboard_handler();
        }
    }
}