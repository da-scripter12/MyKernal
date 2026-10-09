#ifndef COMMANDS_H
#define COMMANDS_H

extern char command[128];
extern int command_length;

int string_equals(const char *a, const char *b);
int starts_with(const char *text, const char *prefix);
void execute_command(void);

#endif
