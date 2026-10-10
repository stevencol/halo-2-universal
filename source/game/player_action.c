/*
PLAYER_ACTION.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

What a player does in a tick of the game. The name of the file is a guess,
and so are the names of its functions.
*/

#include "player_action.h"

#include "math/real_math.h"

#include <string.h>

/* ---------- code */

/* 0x0014d040: what a player does is nothing: all zeros, but for the numbers
that are NONE when there is nothing */
void player_action_clear(
	byte *action)
{
	memset(action, 0, PLAYER_ACTION_SIZE);
	*(short *)(action+PLAYER_ACTION_UNKNOWN1E_OFFSET)= NONE;
	action[PLAYER_ACTION_UNKNOWN20_OFFSET]= (byte)NONE;
	action[PLAYER_ACTION_UNKNOWN21_OFFSET]= (byte)NONE;
	*(short *)(action+PLAYER_ACTION_UNKNOWN22_OFFSET)= NONE;
	*(short *)(action+PLAYER_ACTION_UNKNOWN24_OFFSET)= NONE;
	*(short *)(action+PLAYER_ACTION_UNKNOWN4C_OFFSET)= 0;
	*(real *)(action+PLAYER_ACTION_UNKNOWN50_OFFSET)= 0.0f;
	*(real *)(action+PLAYER_ACTION_UNKNOWN54_OFFSET)= 0.0f;
	*(long *)(action+PLAYER_ACTION_UNKNOWN34_OFFSET)= NONE;
	*(long *)(action+PLAYER_ACTION_UNKNOWN38_OFFSET)= NONE;
	*(long *)(action+PLAYER_ACTION_UNKNOWN3C_OFFSET)= NONE;
	*(long *)(action+PLAYER_ACTION_UNKNOWN28_OFFSET)= 0;
	*(long *)(action+PLAYER_ACTION_UNKNOWN2C_OFFSET)= NONE;
	*(long *)(action+PLAYER_ACTION_UNKNOWN30_OFFSET)= NONE;
}

/* whether a real is a number that has an end: not one of the infinities,
nor one that is no number */
static inline __attribute__((always_inline)) boolean player_action_real_finite(
	const byte *action,
	long offset)
{
	return (*(const unsigned long *)(action+offset)&0x7F800000)!=0x7F800000;
}

/* 0x0014d0a0: whether what a player does is good: there is something; its
first six reals have an end, the one at 0x14 is from 0 to 1, and the ones at
0xC and 0x10 from -1 to 1; the two bytes at 0x20 are NONE or from 0 to 3, and
not the same unless NONE; the number at 0x22 is NONE or less than 2, and the
one at 0x24 NONE or less than 4; the byte at 0x1D has no bits past the
first; the two reals at 0x50 are from 0 to 1, and if one of them is not
nothing, or the first bit at 0x4C is set, the number at 0x34 is not NONE;
and the number at 0x28 is from 0 to 8, with the one after it not NONE unless
it is nothing */
boolean player_action_valid(
	const byte *action)
{
	signed char first;
	signed char second;
	short number;
	real unknown50;
	real unknown54;

	if (!action)
	{
		return FALSE;
	}
	if (!player_action_real_finite(action, PLAYER_ACTION_UNKNOWN8_OFFSET) ||
		!player_action_real_finite(action, PLAYER_ACTION_UNKNOWN4_OFFSET) ||
		!player_action_real_finite(action, PLAYER_ACTION_UNKNOWN14_OFFSET) ||
		!player_action_real_finite(action, PLAYER_ACTION_UNKNOWN18_OFFSET))
	{
		return FALSE;
	}
	if (!(*(const real *)(action+PLAYER_ACTION_UNKNOWN14_OFFSET)>=0.0f) ||
		!(1.0f>=*(const real *)(action+PLAYER_ACTION_UNKNOWN14_OFFSET)))
	{
		return FALSE;
	}
	if (!player_action_real_finite(action, PLAYER_ACTION_UNKNOWNC_OFFSET) ||
		!player_action_real_finite(action, PLAYER_ACTION_UNKNOWN10_OFFSET))
	{
		return FALSE;
	}
	if (!(*(const real *)(action+PLAYER_ACTION_UNKNOWNC_OFFSET)>=-1.0f) ||
		!(1.0f>=*(const real *)(action+PLAYER_ACTION_UNKNOWNC_OFFSET)) ||
		!(*(const real *)(action+PLAYER_ACTION_UNKNOWN10_OFFSET)>=-1.0f) ||
		!(1.0f>=*(const real *)(action+PLAYER_ACTION_UNKNOWN10_OFFSET)))
	{
		return FALSE;
	}
	first= (signed char)action[PLAYER_ACTION_UNKNOWN20_OFFSET];
	if (first!=NONE && (first<0 || first>=4))
	{
		return FALSE;
	}
	second= (signed char)action[PLAYER_ACTION_UNKNOWN21_OFFSET];
	if (second!=NONE && (second<0 || second>=4))
	{
		return FALSE;
	}
	if (first!=NONE && first==second)
	{
		return FALSE;
	}
	number= *(const short *)(action+PLAYER_ACTION_UNKNOWN22_OFFSET);
	if (number!=NONE && (number<0 || number>=2))
	{
		return FALSE;
	}
	number= *(const short *)(action+PLAYER_ACTION_UNKNOWN24_OFFSET);
	if (number!=NONE && (number<0 || number>=4))
	{
		return FALSE;
	}
	if (action[PLAYER_ACTION_UNKNOWN1D_OFFSET]&0xFE)
	{
		return FALSE;
	}
	unknown50= *(const real *)(action+PLAYER_ACTION_UNKNOWN50_OFFSET);
	unknown54= *(const real *)(action+PLAYER_ACTION_UNKNOWN54_OFFSET);
	if (!(unknown50>=0.0f && 1.0f>=unknown50 && unknown54>=0.0f && 1.0f>=unknown54))
	{
		return FALSE;
	}
	if (!(unknown50==0.0f && unknown54==0.0f && !(action[PLAYER_ACTION_UNKNOWN4C_OFFSET]&1)) &&
		*(const long *)(action+PLAYER_ACTION_UNKNOWN34_OFFSET)==NONE)
	{
		return FALSE;
	}
	number= *(const short *)(action+PLAYER_ACTION_UNKNOWN28_OFFSET);
	if (number<0 || number>=9)
	{
		return FALSE;
	}
	if (number!=0 && *(const long *)(action+PLAYER_ACTION_UNKNOWN2C_OFFSET)==NONE)
	{
		return FALSE;
	}

	return TRUE;
}
