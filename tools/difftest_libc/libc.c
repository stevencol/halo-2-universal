/*
LIBC.C

The C library functions of include/, for the image that the function tests
run. The image has no other library.
*/

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <wctype.h>

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

char *strchr(const char *string, int character)
{
	for (; ; string++)
	{
		if (*string==(char)character)
		{
			return (char *)string;
		}
		if (!*string)
		{
			return NULL;
		}
	}
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

char *strncpy(char *destination, const char *source, size_t size)
{
	size_t i;

	for (i= 0; i<size && source[i]; i++)
	{
		destination[i]= source[i];
	}
	for (; i<size; i++)
	{
		destination[i]= 0;
	}

	return destination;
}

char *strrchr(const char *string, int character)
{
	const char *last= NULL;

	do
	{
		if (*string==(char)character)
		{
			last= string;
		}
	} while (*string++);

	return (char *)last;
}

size_t strspn(const char *string, const char *characters)
{
	size_t length= 0;

	while (string[length] && memchr(characters, string[length], strlen(characters)))
	{
		length++;
	}

	return length;
}

char *strpbrk(const char *string, const char *characters)
{
	for (; *string; string++)
	{
		if (memchr(characters, *string, strlen(characters)))
		{
			return (char *)string;
		}
	}

	return NULL;
}

/* as the game's, with no locale set */
wint_t towlower(wint_t character)
{
	return character>='A' && character<='Z' ? character+('a'-'A') : character;
}

/* writes a character if it fits, and counts it */
static void print_character(char *buffer, size_t size, size_t *length, char character)
{
	if (*length<size)
	{
		buffer[*length]= character;
	}
	(*length)++;
}

/* as the game's (_vsnprintf): text that does not fit is cut with no zero
after it, and the result is then -1 */
int vsnprintf(char *buffer, size_t size, const char *format, va_list arguments)
{
	size_t length= 0;

	for (; *format; format++)
	{
		char digits[16];
		const char *text= digits;
		size_t text_length= 0;
		size_t width= 0;
		size_t precision= (size_t)-1;
		int left= 0;
		int zero= 0;
		int negative= 0;

		if (*format!='%')
		{
			print_character(buffer, size, &length, *format);
			continue;
		}
		format++;
		for (; *format=='-' || *format=='0'; format++)
		{
			if (*format=='-')
			{
				left= 1;
			}
			else
			{
				zero= 1;
			}
		}
		for (; *format>='0' && *format<='9'; format++)
		{
			width= width*10+(size_t)(*format-'0');
		}
		if (*format=='.')
		{
			precision= 0;
			for (format++; *format>='0' && *format<='9'; format++)
			{
				precision= precision*10+(size_t)(*format-'0');
			}
		}
		while (*format=='l' || *format=='h')
		{
			format++;
		}

		switch (*format)
		{
		case 's':
			text= va_arg(arguments, const char *);
			if (!text)
			{
				text= "(null)";
			}
			while (text_length<precision && text[text_length])
			{
				text_length++;
			}
			zero= 0;
			break;
		case 'c':
			digits[0]= (char)va_arg(arguments, int);
			text_length= 1;
			zero= 0;
			break;
		case 'd':
		case 'i':
		case 'u':
		case 'x':
		case 'X':
		{
			unsigned int value= va_arg(arguments, unsigned int);
			unsigned int base= (*format=='x' || *format=='X') ? 16 : 10;
			const char *numerals= *format=='X' ? "0123456789ABCDEF" : "0123456789abcdef";
			char reversed[16];
			size_t count= 0;

			if ((*format=='d' || *format=='i') && (int)value<0)
			{
				negative= 1;
				value= 0u-value;
			}
			do
			{
				reversed[count++]= numerals[value%base];
				value/= base;
			} while (value);
			while (count)
			{
				digits[text_length++]= reversed[--count];
			}
			break;
		}
		case '%':
			digits[0]= '%';
			text_length= 1;
			zero= 0;
			break;
		default:
			/* not a conversion that this has: the rest of the format is not written */
			goto done;
		}

		{
			size_t padding= width>text_length+(size_t)negative ? width-text_length-(size_t)negative : 0;
			size_t i;

			if (!left && !zero)
			{
				for (i= 0; i<padding; i++)
				{
					print_character(buffer, size, &length, ' ');
				}
			}
			if (negative)
			{
				print_character(buffer, size, &length, '-');
			}
			if (!left && zero)
			{
				for (i= 0; i<padding; i++)
				{
					print_character(buffer, size, &length, '0');
				}
			}
			for (i= 0; i<text_length; i++)
			{
				print_character(buffer, size, &length, text[i]);
			}
			if (left)
			{
				for (i= 0; i<padding; i++)
				{
					print_character(buffer, size, &length, ' ');
				}
			}
		}
	}

done:
	if (length<size)
	{
		buffer[length]= 0;
	}

	return length>size ? -1 : (int)length;
}

/* the compiler uses the processor's instructions for these where it can;
these are for where it calls them */
#undef sqrtf
#undef fabsf

float sqrtf(float value)
{
	__asm__("fsqrt" : "+t"(value));

	return value;
}

float fabsf(float value)
{
	__asm__("fabs" : "+t"(value));

	return value;
}

/* the game takes sines and cosines with the processor's instructions */
float sinf(float value)
{
	__asm__("fsin" : "+t"(value));

	return value;
}

float cosf(float value)
{
	__asm__("fcos" : "+t"(value));

	return value;
}

float atan2f(float y, float x)
{
	/* fpatan takes x from the top of the x87 stack and y from under it */
	__asm__("fpatan" : "+t"(x) : "u"(y) : "st(1)");

	return x;
}

long lrintf(float value)
{
	long result;

	__asm__("cvtss2si %1, %0" : "=r"(result) : "x"(value));

	return result;
}

int strncmp(const char *a, const char *b, size_t size)
{
	for (; size && *a && *a==*b; size--, a++, b++)
	{
	}

	return !size ? 0 : (unsigned char)*a<(unsigned char)*b ? -1 : (unsigned char)*a>(unsigned char)*b;
}

/* with characters of 16 bits, as the game's */
unsigned short *wcsncpy(unsigned short *destination, const unsigned short *source, size_t count)
{
	size_t index= 0;

	for (; index<count && source[index]; index++)
	{
		destination[index]= source[index];
	}
	for (; index<count; index++)
	{
		destination[index]= 0;
	}

	return destination;
}

/* what the compiler calls to divide numbers of 64 bits, which the processor
does not do: the quotient, towards zero */
static unsigned long long divide_unsigned(unsigned long long dividend, unsigned long long divisor)
{
	unsigned long long quotient= 0;
	unsigned long long remainder= 0;
	int bit;

	for (bit= 63; bit>=0; bit--)
	{
		remainder= (remainder<<1)|((dividend>>bit)&1);
		if (remainder>=divisor)
		{
			remainder-= divisor;
			quotient|= 1ULL<<bit;
		}
	}

	return quotient;
}

long long __divdi3(long long dividend, long long divisor)
{
	int negative= (dividend<0)!=(divisor<0);
	unsigned long long quotient= divide_unsigned(
		dividend<0 ? 0ULL-(unsigned long long)dividend : (unsigned long long)dividend,
		divisor<0 ? 0ULL-(unsigned long long)divisor : (unsigned long long)divisor);

	return (long long)(negative ? 0ULL-quotient : quotient);
}

int wcsncmp(const unsigned short *a, const unsigned short *b, size_t count)
{
	if (!count)
	{
		return 0;
	}
	while (--count && *a && *a==*b)
	{
		a++;
		b++;
	}

	return (int)*a-(int)*b;
}
