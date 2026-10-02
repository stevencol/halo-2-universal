/*
HASH_TABLE.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

A table that finds things by their keys. The names of the file and of the
functions are guesses.
*/

#include "hash_table.h"

#include <string.h>

/* ---------- code */

static inline __attribute__((always_inline)) struct hash_table_entry **hash_table_get_lists(
	struct hash_table *table)
{
	return (struct hash_table_entry **)(table+1);
}

/* 0x0013e1a0: a table for so many things at most, each with so many bytes
with its key, in so many lists. NULL if there is no memory for it */
struct hash_table *hash_table_new(
	struct allocation *allocation,
	long maximum_count,
	const char *name,
	long size,
	long list_count,
	hash_table_hash_proc hash,
	hash_table_compare_proc compare)
{
	struct hash_table *table= allocation->functions->allocate(allocation,
		(unsigned long)((size+(long)sizeof(struct hash_table_entry))*maximum_count + list_count*(long)sizeof(void *) +
		(long)sizeof(struct hash_table)));

	if (table)
	{
		strncpy(table->name, name, HASH_TABLE_NAME_LENGTH+1);
		table->compare= compare;
		table->name[HASH_TABLE_NAME_LENGTH]= 0;
		table->list_count= list_count;
		table->maximum_count= maximum_count;
		table->hash= hash;
		table->allocation= allocation;
		table->size= size;
		hash_table_clear(table);
	}

	return table;
}

/* 0x0013e210: nothing is in the table: all of its entries are free */
void hash_table_clear(
	struct hash_table *table)
{
	unsigned long entry_size= (unsigned long)table->size+sizeof(struct hash_table_entry);
	unsigned long lists_size= (unsigned long)table->list_count*sizeof(void *);
	struct hash_table_entry *entry;
	long count;

	memset(hash_table_get_lists(table), 0, entry_size*(unsigned long)table->maximum_count + lists_size);
	entry= (struct hash_table_entry *)((byte *)hash_table_get_lists(table)+lists_size);
	table->first_free= NULL;
	for (count= table->maximum_count; count>0; count--)
	{
		entry->next= table->first_free;
		table->first_free= entry;
		entry= (struct hash_table_entry *)((byte *)entry+entry_size);
	}
}

/* 0x0013e270: a thing more in the table, with what is kept with its key.
Whether there was room for it. It is not seen whether the key was there */
boolean hash_table_add(
	struct hash_table *table,
	long key,
	const void *data)
{
	struct hash_table_entry *entry;
	unsigned long hash;
	unsigned long list;

	if (!table->first_free)
	{
		return FALSE;
	}
	hash= table->hash(key);
	list= hash%(unsigned long)table->list_count;
	entry= table->first_free;
	table->first_free= entry->next;
	entry->hash= hash;
	entry->key= key;
	memcpy(entry+1, data, (unsigned long)table->size);
	entry->next= hash_table_get_lists(table)[list];
	hash_table_get_lists(table)[list]= entry;

	return TRUE;
}

/* 0x0013e2d0: the thing of a key; NULL if it is not in the table */
struct hash_table_entry *hash_table_find(
	struct hash_table *table,
	long key)
{
	unsigned long hash= table->hash(key);
	struct hash_table_entry *entry;

	for (entry= hash_table_get_lists(table)[hash%(unsigned long)table->list_count]; entry; entry= entry->next)
	{
		if (hash==entry->hash && table->compare(key, entry->key))
		{
			return entry;
		}
	}

	return NULL;
}

/* 0x0013e320: a thing less in the table. Whether it was there. The hashes
are not compared here: only the keys */
boolean hash_table_remove(
	struct hash_table *table,
	long key)
{
	unsigned long list= table->hash(key)%(unsigned long)table->list_count;
	struct hash_table_entry *previous= NULL;
	struct hash_table_entry *entry;

	for (entry= hash_table_get_lists(table)[list]; entry; entry= entry->next)
	{
		if (table->compare(key, entry->key))
		{
			if (previous)
			{
				previous->next= entry->next;
			}
			else
			{
				hash_table_get_lists(table)[list]= entry->next;
			}
			entry->next= table->first_free;
			table->first_free= entry;

			return TRUE;
		}
		previous= entry;
	}

	return FALSE;
}
