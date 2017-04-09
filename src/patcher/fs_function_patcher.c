/****************************************************************************
 * Copyright (C) 2016 Maschell
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/

#include "fs_function_patcher.h"
#include "menu_icon.h"
#include "utils/logger.h"
#include <string.h>

int tga_fd[255];
int array_size = 0;

DECL(int, FSOpenFile, void *pClient, void *pCmd, const char *path, const char *mode, int *fd, int errHandling)
{
	char * file_name = (char *)((u32)strrchr(path, '/') + 1);
    if (strcmp(file_name, "iconTex.tga") == 0)
	{
		// Save the fd in the fd array
		int temp_fd;
		int ret = real_FSOpenFile(pClient, pCmd, path, mode, &temp_fd, errHandling);

		tga_fd[array_size] = temp_fd;
		array_size++;
		
		*fd = temp_fd;
		
		return ret;
	}
	
	return real_FSOpenFile(pClient, pCmd, path, mode, fd, errHandling);
}

DECL(int, FSGetStat, void *pClient, void *pCmd, const char *path, FSStat *stats, int errHandling)
{
	char * file_name = (char *)((u32)strrchr(path, '/') + 1);
    if (strcmp(file_name, "iconTex.tga") == 0)
	{
		int ret = real_FSGetStat(pClient, pCmd, path, stats, errHandling);
		stats->size = menu_icon_len;
		stats->alloc_size = menu_icon_len;
		return ret;
	}
	
	return real_FSGetStat(pClient, pCmd, path, stats, errHandling);
}

DECL(int, FSReadFile, void *pClient, void *pCmd, void *buffer, int size, int count, int fd, int flag, int errHandling)
{
	for(int i = 0; i < array_size; i++)
	{
		if (fd == tga_fd[i])
		{
			// The fd is in the list of TGAs fds
			memcpy(buffer, menu_icon, menu_icon_len);
			return menu_icon_len;
		}
	}
	
	return real_FSReadFile(pClient, pCmd, buffer, size, count, fd, flag, errHandling);
}

DECL(int, FSGetStatFile, void *pClient, void *pCmd, int fd, FSStat *stat, int error)
{
	for(int i = 0; i < array_size; i++)
	{
		// The fd is in the list of TGAs fds
		if (fd == tga_fd[i])
		{
			int ret = real_FSGetStatFile(pClient, pCmd, fd, stat, error);
			stat->size = menu_icon_len;
			stat->alloc_size = menu_icon_len;
			return ret;
		}
	}
	
	return real_FSGetStatFile(pClient, pCmd, fd, stat, error);
}


hooks_magic_t method_hooks_fs[] __attribute__((section(".data"))) = {
	MAKE_MAGIC(FSOpenFile,		LIB_CORE_INIT,	STATIC_FUNCTION),
	MAKE_MAGIC(FSGetStat,		LIB_CORE_INIT,	STATIC_FUNCTION),
	MAKE_MAGIC(FSReadFile,		LIB_CORE_INIT,	STATIC_FUNCTION),
	MAKE_MAGIC(FSGetStatFile,	LIB_CORE_INIT,	STATIC_FUNCTION),
	
};

u32 method_hooks_size_fs __attribute__((section(".data"))) = sizeof(method_hooks_fs) / sizeof(hooks_magic_t);

//! buffer to store our instructions needed for our replacements
volatile unsigned int method_calls_fs[sizeof(method_hooks_fs) / sizeof(hooks_magic_t) * FUNCTION_PATCHER_METHOD_STORE_SIZE] __attribute__((section(".data")));

