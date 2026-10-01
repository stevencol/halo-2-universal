/*
ASYNC.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.
*/

#include "async.h"

/* ---------- constants */

#define INFINITE 0xFFFFFFFF

/* ---------- globals */

/* 0x004e035c: the mutex of the list of tasks */
uint32_t async_mutex;

/* 0x004e28ec: the tasks that are not done yet */
struct async_task *async_task_list;

/* ---------- code */

/* 0x00120b50: whether a task of a category is not done yet. The name is a
guess */
boolean async_category_busy(
	long category)
{
	boolean busy= FALSE;
	struct async_task *task;

	WaitForSingleObject(async_mutex, INFINITE);
	for (task= async_task_list; task; task= task->next)
	{
		if (task->category==category)
		{
			busy= TRUE;
			break;
		}
	}
	ReleaseMutex(async_mutex);

	return busy;
}
