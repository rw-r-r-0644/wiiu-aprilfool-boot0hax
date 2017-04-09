#include "font.h"
#include "tty.h"
#include "system/memory.h"
#include "dynamic_libs/os_functions.h"

#include <stdio.h>
#include <string.h>


/* Definitions */
#define BUF_ADDR_0 (u32*)0xF4000000
#define BUF_ADDR_1 (u32*)0xF4708000

#define BUF_SIZE_0 0x00708000
#define BUF_SIZE_1 0x00348000

#define BUFFER_SIDE_OFFSET 0x69000

#define CHAR_SIZE_X (8)
#define CHAR_SIZE_Y (8)

/* Globals */
u32* framebuffer = BUF_ADDR_1;
u32  framebuffer_size = BUF_SIZE_1;

int line = 0;

/* Screen functions */
void tty_clear(u32 color)
{
	OSScreenClearBufferEx(1, color);
	line = 0;
}

void tty_flip()
{
	DCFlushRange(framebuffer, framebuffer_size);
    OSScreenFlipBuffersEx(1);
}

/* Char drawing (that comes from iosuhax) */
void tty_draw_char(char c, int x, int y)
{
	if(c < 32)
		return;
	c -= 32;
	
	u8* charData = (u8*)&font_bin[(CHAR_SIZE_X * CHAR_SIZE_Y * c) / 8];
	
	u32 * fb = &framebuffer[x + y * 896];

	int i, j;
	for(i = 0; i < CHAR_SIZE_Y; i++)
	{
		u8 v= *(charData++);
		for(j = 0; j < CHAR_SIZE_X; j++)
		{
			if(v & 1)
			{
				fb[0] = 0x00000000;
				fb[0x69000] = 0x00000000;
			}
			else
			{
				fb[0] = 0xFFFFFFFF;
				fb[0x69000] = 0xFFFFFFFF;
			}
			v >>= 1;
			fb++;
		}
		fb += 896 - CHAR_SIZE_X;
	}
}

/* Print to location functions */
void tty_print_at(int x, int y, char* str)
{
	if(!str)
		return;
	
	int k;
	int dx = 0, dy = 0;
	for(k = 0; str[k]; k++)
	{
		if(str[k] >= 32 && str[k] < 128)
			tty_draw_char(str[k], x + dx, y + dy);
		
		dx += 8;
			
		if(str[k] == '\n')
		{
			dx = 0;
			dy += 8;
		}
	}
	
	tty_flip(); // Update the screen
}

void tty_printf_at(int x, int y, const char *format, ...)
{
    va_list args;
    va_start(args, format);

    static char buffer[0x100];

    vsnprintf(buffer, 0xFF, format, args);
    tty_print_at(x, y, buffer);

    va_end(args);
}

/* Print functions wrappers */
void tty_scroll()
{
	// Scroll the buffer of 8 pixels
	memcpy(&framebuffer[0x00000], &framebuffer[0x01C00], 0x19D000);
	memcpy(&framebuffer[0x69000], &framebuffer[0x6AC00], 0x19D000);
	
	// Clear the last 8 pixel lines
	memset(&framebuffer[0x67400], 0, 0x7000);
	memset(&framebuffer[0xD0400], 0, 0x7000);
	
	tty_flip();
}

void tty_print(char* str)
{
	if(!str)
		return;
	
	if (line > 59)
	{
		tty_scroll();
		line = 59;
	}
	
	int k;
	int dx = 0, dy = 0;
	for(k = 0; str[k]; k++)
	{
		// ---start of bad part---
		// That part parses "\n" in the text to fix the BAD encryption, remove it when using this libary
		if(str[k] == '\\' && str[k + 1] == 'n')
		{
			k++;
			dx = 0;
			dy += 8;
			
			line++;
			if (line > 59)
			{
				tty_scroll();
				line = 59;
			}
			continue;
		}
		// ---end of bad part---
		
		if(str[k] >= 32 && str[k] < 128)
			tty_draw_char(str[k], dx, (line * 8) + dy);
		
		dx += 8;
		
		// Yeah, uhm, I'm not sure that this should be parsed there but either way...
		if(str[k] == '\n')
		{
			dx = 0;
			dy += 8;
			
			line++;
			if (line > 59)
			{
				tty_scroll();
				line = 59;
			}
		}
	}
	
	tty_flip(); // Update the screen
	
	line++;
}

void tty_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    static char buffer[0x100];

    vsnprintf(buffer, 0xFF, format, args);
    tty_print(buffer);

    va_end(args);
}

void tty_newline()
{
	if (line > 59)
	{
		tty_scroll();
		line = 59;
	}
	line++;
}

/* Initialization and deinitialization */
void tty_init()
{
    OSScreenInit();
    OSScreenSetBufferEx(1, framebuffer);
    OSScreenEnableEx(1, 1);
    tty_clear(0x00000000);
}

void tty_end()
{
	tty_clear(0x00000000);
	tty_flip();
	tty_clear(0x00000000);
	tty_flip();
}


/*
// This could be edited to find tv second buffer, if you need it
void find_buffers()
{
	//OSScreenPutPixelEx(1, x + j, y + i, (v & 1) ? 0x00000000 : 0xFFFFFFFF);
	OSScreenPutPixelEx(1, 0, 0, 0xDEADBEEF);
	tty_flip();
	OSScreenPutPixelEx(1, 0, 0, 0xCAFEDEAD);

	u32 *screen = BUF_ADDR_1;
	
	u32 off1 = 0;
	u32 off2 = 0;
	
	for (u32 i = 0; i < ((BUF_SIZE_1 + 0x1000000) / 4); i++)
	{
		if (screen[i] == 0xDEADBEEF)
			off1 = i;
		else if (screen[i] == 0xCAFEDEAD)
			off2 = i;
	}
	
	tty_printf("off1: %08x", off1);
	tty_printf("off2: %08x", off2);
}

void put_pixel(int x, int y, u32 color)
{
	//Code to write to framebuffer directly
	u32 *screen = BUF_ADDR_1;
	u32 v = (x + y * 896);
	// Fixed direct pixel write code from libwiiu, 0x69000 is the offset for gamepad's second buffer :)
	screen[v] = color;
	screen[v + 0x69000] = color;
	
	// We should care about tv too but lazy... :P
}
*/
