/*
ONLINE_TEAMS.H

The teams of the players in the service of games by the network (what the
game shows as clans, most likely): the online tasks that ask for them and
change them. Worked out from halo2ship.exe; the name of the file is a guess,
and so are the names of its functions.
*/

#ifndef __ONLINE_TEAMS_H
#define __ONLINE_TEAMS_H

#include "cseries/cseries.h"
#include "networking/online_tasks.h"

/* ---------- constants */

enum
{
	/* the kinds of online task of this file */
	_online_task_type_teams_enumerate= 0x17,
	_online_task_type_team_unknown18,
	_online_task_type_team_unknown19,
	_online_task_type_team_unknown1D= 0x1D,
	_online_task_type_team_unknown1F= 0x1F,
	_online_task_type_team_members_enumerate,
	/* how big what the libraries say of a team and of one of its members
	is, and where the bits of a member are */
	ONLINE_TEAM_SIZE= 0x6A2,
	ONLINE_TEAM_MEMBER_SIZE= 0x92,
	ONLINE_TEAM_MEMBER_FLAGS_OFFSET= 0x1C,
	/* a number that the libraries are given with something of a team, and
	the size of what goes with it */
	ONLINE_TEAM_UNKNOWN_000ABE60_NUMBER= 100,
	ONLINE_TEAM_UNKNOWN_000ABE60_SIZE= 0x6C,
	/* the three answers of online_team_unknown_000abfa0_start */
	NUMBER_OF_ONLINE_TEAM_ANSWERS= 3
};

/* ---------- structures */

/* who someone is in the service of games, or which team: a number of 64
bits and some bits */
struct online_xuid
{
	unsigned long id[2];
	unsigned long flags;
};

/* ---------- prototypes/ONLINE_TEAMS.C */

long __attribute__((stdcall)) online_teams_enumerate_start(const struct online_xuid *user, long owner);
void __attribute__((stdcall)) online_teams_enumerate_get_results(long task_index, long *count,
	struct online_xuid *teams);
void __attribute__((stdcall)) online_team_get(long task_index, const struct online_xuid *team, byte *information);
long __attribute__((stdcall)) online_team_unknown_000abe60_start(long owner, long unknown);
long __attribute__((stdcall)) online_team_unknown_000abf10_start(const struct online_xuid *team, long owner);
long __attribute__((stdcall)) online_team_unknown_000abfa0_start(const struct online_xuid *team, long owner,
	long answer);
long __attribute__((stdcall)) online_team_unknown_000ac050_start(const struct online_xuid *unknown, long owner,
	const struct online_xuid *team);
long __attribute__((stdcall)) online_team_members_enumerate_start(const struct online_xuid *team, long owner);
void __attribute__((stdcall)) online_team_members_enumerate_get_results(long task_index, long *count,
	struct online_xuid *members);
void __attribute__((stdcall)) online_team_member_get(long task_index, const struct online_xuid *member,
	byte *information);

#endif
