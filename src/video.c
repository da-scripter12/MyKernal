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
