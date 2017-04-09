#pragma once

#include <stdarg.h>
#include "common/types.h"

void tty_flip();
void tty_clear(u32 color);
void tty_print_at(int x, int y, char* str);
void tty_printf_at(int x, int y, const char *format, ...);
void tty_print(char* str);
void tty_printf(const char *format, ...);
void tty_init();
void tty_end();
