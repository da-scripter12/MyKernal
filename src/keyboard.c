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
