# mutations of milestone 758: one or two lines of each function

TESTS = "test_small_758"

HOPPERS = "source/networking/network_hoppers.c"

MUTATIONS = [
    (HOPPERS, "\tunsigned long numerator= (unsigned long)(maximum_players-1)*(unsigned long)scale;", "\tunsigned long numerator= (unsigned long)maximum_players*(unsigned long)scale;"),
    (HOPPERS, "\t\t\tvalue= (long)numerator/(maximum_players-minimum_players);", "\t\t\tvalue= (long)numerator/(maximum_players-minimum_players+1);"),
    (HOPPERS, "\t\tnumerator-= (unsigned long)scale;", "\t\tnumerator-= (unsigned long)scale*2;"),
    (HOPPERS, "\tlong minimum_players= (long)((unsigned long)team_size*(unsigned long)minimum_teams);", "\tlong minimum_players= (long)((unsigned long)team_size*(unsigned long)minimum_teams)+1;"),
    (HOPPERS, "\t\t\tvalue= (long)((unsigned long)(maximum_teams-player_count/team_size)*(unsigned long)scale)/", "\t\t\tvalue= (long)((unsigned long)(maximum_teams-player_count)*(unsigned long)scale)/"),
    (HOPPERS, "\t\t\t\t(maximum_teams-minimum_teams);", "\t\t\t\t(maximum_teams-minimum_teams+1);"),
    (HOPPERS, "\tfile_path_add_name(file->path, network_hoppers_file_name);", "\tfile_path_add_name(file->path, \"multiplayer_hopper.da\");"),
    (HOPPERS, "\tfile_path_add_name(file->path, network_hoppers_file_name);\n\tfile->flags|= 1;", "\tfile_path_add_name(file->path, network_hoppers_file_name);\n\tfile->flags|= 2;"),
    (HOPPERS, "network_hopper_valid(hopper) && hopper->unknown8==unknown8)", "network_hopper_valid(hopper) && hopper->unknown8!=unknown8)"),
    (HOPPERS, "\tfor (index= 0; index<MAXIMUM_NETWORK_HOPPERS && !found; index++)", "\tfor (index= 0; index<MAXIMUM_NETWORK_HOPPERS; index++)"),
]
