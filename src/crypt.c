#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common/types.h"

// HACKY!
int u16len(u16 * buf)
{
	int len = 0;
	while (buf[len])
		len++;
	return len;
}

// REALLY REALLY REALLY BAD ENCRYPTION FUNCTION
char * decrypt_buffer(u16 * buffer, char * key)
{
	int buffer_len = u16len(buffer);
	
	int key_len = strlen(key);
	// Deobfuscate key
	for (int i = 0; i < key_len; i++)
		key[i]--;
	
	// If the key is smaller than the text, copy the key over until it covers the whole text
	char * key_fixed;
	if (key_len < buffer_len)
	{
		key_fixed = malloc(buffer_len + 1);
		int i;
		for (i = 0; i < (buffer_len / key_len); i++)
			memcpy(&key_fixed[i * key_len], key, key_len);
		memcpy(&key_fixed[i * key_len], key, buffer_len - (i * key_len));
	}
	else
	{
		key_fixed = key;
	}
	
	//Subtract the key
	char * text = malloc(buffer_len + 1);
	for (int i = 0; i < buffer_len; i++)
		text[i] = buffer[i] - key_fixed[i];
	
	// Deobfuscate text
	for (int i = 0; i < buffer_len; i++)
		text[i]--;
	
	text[buffer_len] = 0;

	return text;
}