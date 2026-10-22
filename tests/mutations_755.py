# mutations of milestone 755: one or two lines of each function

TESTS = "test_small_755"

LIFE = "source/networking/network_life_cycle.c"

MUTATIONS = [
    (LIFE, "\treturn network_session_leader_set_unknown5dd0(session, unknown5dd0) &&", "\treturn network_session_leader_set_unknown5dd0(session, 0) &&"),
    (LIFE, "\treturn network_session_leader_set_unknown5dd0(session, unknown5dd0) &&\n\t\tnetwork_session_unknown_0005bf10(session, FALSE, 0, 0, NULL);", "\treturn network_session_leader_set_unknown5dd0(session, unknown5dd0);"),
    (LIFE, "\tif (unknown_00527330 && unknown_00527334==_network_life_cycle_state3)", "\tif (unknown_00527330 && unknown_00527334==_network_life_cycle_state4)"),
    (LIFE, "\t\t\t\t*(const long *)(session+NETWORK_SESSION_HOST_PEER_OFFSET) &&\n\t\t\tnetwork_session_leader_set_unknown49c4(session))", "\t\t\t\t*(const long *)(session+NETWORK_SESSION_HOST_PEER_OFFSET))"),
    (LIFE, "\t\t\tconst byte *profile= unknown_004cd904[profile_index];", "\t\t\tconst byte *profile= unknown_004cd904[0];"),
    (LIFE, "network_session_unknown_0005bf10(session, unknown8, unknownC, unknown10, profile+1))", "network_session_unknown_0005bf10(session, unknown8, unknown10, unknownC, profile+1))"),
    (LIFE, "\t\t\treturn network_session_get_unknown4d08(session, unknown4d08, unknown4d0c, unknown4d10);", "\t\t\treturn network_session_get_unknown4d08(session, unknown4d0c, unknown4d08, unknown4d10);"),
    (LIFE, "\t\t\treturn *(const short *)(session+NETWORK_SESSION_UNKNOWN5DD0_OFFSET);", "\t\t\treturn *(const short *)(session+NETWORK_SESSION_UNKNOWN5DD0_OFFSET+2);"),
    (LIFE, "\t\t\tif (!session[NETWORK_SESSION_HAS_IDENTIFIER_OFFSET])", "\t\t\tif (session[NETWORK_SESSION_HAS_IDENTIFIER_OFFSET])"),
    (LIFE, "\t\t\t\tmemcpy(key, session+NETWORK_SESSION_KEY_OFFSET, 0x10);", "\t\t\t\tmemcpy(key, session+NETWORK_SESSION_KEY_OFFSET, 0xC);"),
    (LIFE, "\t\t\t\tmemcpy(identifier, session+NETWORK_SESSION_IDENTIFIER_OFFSET, 8);", "\t\t\t\tmemcpy(identifier, session+NETWORK_SESSION_IDENTIFIER_OFFSET+1, 8);"),
]
