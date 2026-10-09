#include "io.h"
#include "video.h"
#include "keyboard.h"

void kernel_main(void)
{
    clear_screen();

    print("========================================\n");
    print("              GojiOS Kernel\n");
    print("========================================\n\n");
    print("Welcome to GojiOS!\n");
    print("Type 'help' for commands.\n\n");

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
