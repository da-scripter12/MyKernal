#ifndef VIDEO_H
#define VIDEO_H

#include "io.h"

#define KEYBOARD_DATA_PORT   0x60
#define KEYBOARD_STATUS_PORT 0x64
#define VIDEO_MEMORY         0xB8000
#define WIDTH                80
#define HEIGHT               25

void put_char(char c);
void print(const char *text);
void clear_screen(void);
void video_backspace(void);

#endif
