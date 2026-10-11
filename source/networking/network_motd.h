/*
NETWORK_MOTD.H

The message of the day: a file of the hard disk that one of the blocks of
the service of games fills (unknown_00479780 of online_tasks.c). Worked out
from halo2ship.exe; the name of the file is a guess, and so are the names of
its functions.
*/

#ifndef __NETWORK_MOTD_H
#define __NETWORK_MOTD_H

#include "cseries/cseries.h"
#include "tag_files/files.h"

/* ---------- prototypes/NETWORK_MOTD.C */

void __attribute__((stdcall)) network_motd_get_file(struct file_reference *file);
boolean network_motd_available(void);

/* ---------- globals */

extern const char *network_motd_file_name;

#endif
