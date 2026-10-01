/*
LIBC.C

The C library functions of include/, for the image that the function tests
run. The image has no other library.
*/

#include <string.h>

/* the linker wants an entry point; nothing calls it */
void _start(void)
{
}

void *memchr(const void *memory, int value, size_t size)
{
	const unsigned char *byte= memory;

	for (; size; size--, byte++)
	{
		if (*byte==(unsigned char)value)
		{
			return (void *)byte;
		}
	}

	return NULL;
}

int memcmp(const void *a, const void *b, size_t size)
{
	const unsigned char *byte_a= a;
	const unsigned char *byte_b= b;

	for (; size; size--, byte_a++, byte_b++)
	{
		if (*byte_a!=*byte_b)
		{
			return *byte_a<*byte_b ? -1 : 1;
		}
	}

	return 0;
}

void *memmove(void *destination, const void *source, size_t size)
{
	unsigned char *to= destination;
	const unsigned char *from= source;

	if (to<from)
	{
		for (; size; size--)
		{
			*to++= *from++;
		}
	}
	else
	{
		for (; size; size--)
		{
			to[size-1]= from[size-1];
		}
	}

	return destination;
}

void *memcpy(void *destination, const void *source, size_t size)
{
	return memmove(destination, source, size);
}

void *memset(void *memory, int value, size_t size)
{
	unsigned char *byte= memory;

	for (; size; size--)
	{
		*byte++= (unsigned char)value;
	}

	return memory;
}

int strcmp(const char *a, const char *b)
{
	for (; *a && *a==*b; a++, b++)
	{
	}

	return (unsigned char)*a<(unsigned char)*b ? -1 : (unsigned char)*a>(unsigned char)*b;
}

size_t strlen(const char *string)
{
	size_t length= 0;

	while (string[length])
	{
		length++;
	}

	return length;
}

int strncmp(const char *a, const char *b, size_t size)
{
	for (; size && *a && *a==*b; size--, a++, b++)
	{
	}

	return !size ? 0 : (unsigned char)*a<(unsigned char)*b ? -1 : (unsigned char)*a>(unsigned char)*b;
}
