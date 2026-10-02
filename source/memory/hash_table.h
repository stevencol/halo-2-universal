/*
HASH_TABLE.H

A table that finds things by their keys: each key has a number, its hash, and
the things whose hashes are the same but for the size of the table are in a
list. The names are guesses.
*/

#ifndef __HASH_TABLE_H
#define __HASH_TABLE_H

#include "cseries/cseries.h"
#include "memory/data.h"

/* ---------- constants */

enum
{
	HASH_TABLE_NAME_LENGTH= 31
};

/* ---------- types */

/* the game's functions that are given to other functions take their
arguments off the stack themselves */
#if defined(__i386__)
#define HASH_TABLE_CALLBACK __attribute__((stdcall))
#else
#define HASH_TABLE_CALLBACK
#endif

typedef unsigned long (HASH_TABLE_CALLBACK *hash_table_hash_proc)(long key);
/* whether two keys are the same */
typedef boolean (HASH_TABLE_CALLBACK *hash_table_compare_proc)(long key, long other);

/* ---------- structures */

/* a thing of the table: after it is what is kept with the key, of the size
that the table says */
struct hash_table_entry
{
	long key;
	unsigned long hash;
	/* in the list of those of its hash, or in that of those that are free */
	struct hash_table_entry *next;
};

/* after it are the lists, and after them all of the entries */
struct hash_table
{
	char name[HASH_TABLE_NAME_LENGTH+1];
	long list_count;
	long maximum_count;
	/* of what is kept with each key */
	long size;
	hash_table_hash_proc hash;
	hash_table_compare_proc compare;
	struct allocation *allocation;
	struct hash_table_entry *first_free;
};

typedef char verify_hash_table_size[
	sizeof(struct hash_table) == 0x3C ? 1 : -1];
typedef char verify_hash_table_entry_size[
	sizeof(struct hash_table_entry) == 0xC ? 1 : -1];

/* ---------- prototypes/HASH_TABLE.C */

struct hash_table *hash_table_new(struct allocation *allocation, long maximum_count, const char *name, long size,
	long list_count, hash_table_hash_proc hash, hash_table_compare_proc compare);
void hash_table_clear(struct hash_table *table);
boolean hash_table_add(struct hash_table *table, long key, const void *data);
struct hash_table_entry *hash_table_find(struct hash_table *table, long key);
boolean hash_table_remove(struct hash_table *table, long key);

#endif
