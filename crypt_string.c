#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

// Bad, quick, encryption code, don't use it for anything else than april fools!
int main(int argc, char *argv[])
{
	char * text = argv[1];
	int text_len = strlen(text);
	
	char key[] = "wait a Minute"; // The time needed for the real boot0 hax xP (don't use [tm], that breaks the code)
	int key_len = strlen(key);
	
	// Do initial text obfuscation
	for (int i = 0; i < text_len; i++)
		text[i]++;
	
	// If the key is smaller than the text, copy the key over until it covers the whole text (yeah, it's a bad solution...)
	char * key_fixed;
	if (key_len < text_len)
	{
		key_fixed = malloc(text_len + 1);
		int i;
		for (i = 0; i < (text_len / key_len); i++)
			memcpy(&key_fixed[i * key_len], key, key_len);
		memcpy(&key_fixed[i * key_len], key, text_len - (i * key_len));
	}
	else
	{
		key_fixed = key;
	}
	
	//Add the key
	uint16_t * encrypted = malloc(sizeof(uint16_t) * text_len);
	for (int i = 0; i < text_len; i++)
		encrypted[i] = text[i] + key_fixed[i];
	
	// Print the encrypted text	+ null termination
	printf("{{");
	for (int i = 0; i < text_len; i++)
		printf("0x%X, ", encrypted[i]);
	printf("0x00}, %s},\n\n", (argv[2] == NULL) ? "0" : argv[2]);
	
	// Obfuscate the key
	for (int i =  0; i < key_len; i++)
		key[i]++;
	
	// Print the obfuscated key
	//printf("const char * key = {");
	//for (int i =  0; i < key_len; i++)
	//	printf("0x%02X, ", key[i]);
	//
	//printf("\b\b};\n");
	
	return 0;
}