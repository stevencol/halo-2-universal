/*
MATH.H

The C library's mathematics that the game's code uses, for the image that
the function tests run. The compiler does them with the processor's
instructions; libc.c defines them for where it does not.
*/

#ifndef __DIFFTEST_MATH_H
#define __DIFFTEST_MATH_H

float sqrtf(float value);
float fabsf(float value);
float sinf(float value);
float cosf(float value);
float atan2f(float y, float x);
float acosf(float value);
long double acosl(long double value);
long double sinl(long double value);
long double fabsl(long double value);
/* to the nearest integer; to the even one, halfway between two */
long lrintf(float value);

#define sqrtf(value) __builtin_sqrtf(value)
#define fabsf(value) __builtin_fabsf(value)

#endif
