# mutations of milestone 833, written by make_mutations.py

TESTS = "test_small_833"

SOURCE_UNITS_UNITS_C = "source/units/units.c"
SOURCE_NETWORKING_NETWORK_LIFE_CYCLE_C = "source/networking/network_life_cycle.c"
SOURCE_SOUND_SOUND_MANAGER_C = "source/sound/sound_manager.c"

MUTATIONS = [
    (SOURCE_UNITS_UNITS_C, '\tif (unit_index==NONE || weapon_index==NONE)', '\tif (unit_index!=NONE || weapon_index==NONE)'),
    (SOURCE_UNITS_UNITS_C, '\tslot= (signed char)unit[0x212];', ''),
    (SOURCE_UNITS_UNITS_C, '\tcount= held==weapon_index ? unit[0x1FB] : unit[0x1FA];', '\tcount= held==weapon_index ? unit[0x1FB] : unit[0x1fb];'),
    (SOURCE_UNITS_UNITS_C, '\tlong held= NONE;', '\tlong held= 0;'),
    (SOURCE_NETWORKING_NETWORK_LIFE_CYCLE_C, '\t\ttask[0x09]= 0;', '\t\ttask[0x09]= 1;'),
    (SOURCE_NETWORKING_NETWORK_LIFE_CYCLE_C, '\t\ttask[0x08]= 1;', ''),
    (SOURCE_NETWORKING_NETWORK_LIFE_CYCLE_C, '\t\ttask[0x30]= 0;', ''),
    (SOURCE_NETWORKING_NETWORK_LIFE_CYCLE_C, '\t\ttask[0x0A]= 0;', '\t\ttask[0x0A]= 1;'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\tindex= (short)(long)place;', ''),
    # taken out: the place is never below 0 nor the index, and past the last both nearest are the last
]
