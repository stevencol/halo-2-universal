# mutations of milestone 800, written by make_mutations.py

TESTS = "test_small_800"

SOURCE_NETWORKING_SIMULATION_VIEW_C = "source/networking/simulation_view.c"
SOURCE_MEMORY_RING_BUFFER_C = "source/memory/ring_buffer.c"
SOURCE_AI_ACTORS_C = "source/ai/actors.c"

MUTATIONS = [
    (SOURCE_NETWORKING_SIMULATION_VIEW_C, '\tunknown_00510ca4= (unknown_00510ca4+1)%1000;', '\tunknown_00510ca4= (unknown_00510ca4+2)%1000;'),
    (SOURCE_NETWORKING_SIMULATION_VIEW_C, '\tfor (index= 0; index<9; index++)', '\tfor (index= 0; index<9-1; index++)'),
    (SOURCE_NETWORKING_SIMULATION_VIEW_C, '\tif (!unknown_00510ca0 || unknown_00510ca1)', '\tif (!unknown_00510ca0 && unknown_00510ca1)'),
    (SOURCE_NETWORKING_SIMULATION_VIEW_C, '\t\tunknown_00515c34[unknown_00510ca4][index]= entry[index];', ''),
    (SOURCE_NETWORKING_SIMULATION_VIEW_C, '\t\t\tshorts[index]&= 0x7FFF;', '\t\t\tshorts[index]&= 0x8000;'),
    (SOURCE_NETWORKING_SIMULATION_VIEW_C, '\t\t\tshorts[index]= (unsigned short)(0x8000|(values[index]&0x7FFF));', ''),
    (SOURCE_NETWORKING_SIMULATION_VIEW_C, '\tfor (index= 0; index<count; index++)', '\tfor (index= 1; index<count; index++)'),
    (SOURCE_NETWORKING_SIMULATION_VIEW_C, '\t\t\tchanged= TRUE;', ''),
    (SOURCE_NETWORKING_SIMULATION_VIEW_C, '\tdata[4]= NONE;', ''),
    (SOURCE_NETWORKING_SIMULATION_VIEW_C, '\tdata[5]= NONE;', '\tdata[5]= 0;'),
    (SOURCE_NETWORKING_SIMULATION_VIEW_C, '\tdata[2]= NONE;', '\tdata[3]= NONE;'),
    (SOURCE_NETWORKING_SIMULATION_VIEW_C, '\tdata[7]= 0;', '\tdata[8]= 0;'),
    (SOURCE_MEMORY_RING_BUFFER_C, '\tlong last= (offset+size+0xFFF)/0x1000;', '\tlong last= (offset+size+0xFFF)/0x1001;'),
    (SOURCE_MEMORY_RING_BUFFER_C, '\tlong offset= address-*(const long *)(data+0x04);', '\tlong offset= address-*(const long *)(data+0x5);'),
    (SOURCE_MEMORY_RING_BUFFER_C, '\tlong first= offset/0x1000;', '\tlong first= offset/0x1001;'),
    (SOURCE_MEMORY_RING_BUFFER_C, '\t\t((unsigned long *)(data+0x10))[page>>5]&= ~(1UL<<(page&0x1F));', '\t\t((unsigned long *)(data+0x10))[page>>5]&= ~(1UL<<(page&0x20));'),
    (SOURCE_AI_ACTORS_C, '\tif (!(kind&0x83) || (header->flags&0x10) || (object[0x04]&1))', '\tif (!(kind&0x83) || (header->flags&0x11) || (object[0x04]&1))'),
    (SOURCE_AI_ACTORS_C, '\tconst struct object_header_datum *header= (const struct object_header_datum *)object_header_data->data+(object_index&0xFFFF);', '\tconst struct object_header_datum *header= (const struct object_header_datum *)object_header_data->data+(object_index&0x10000);'),
    (SOURCE_AI_ACTORS_C, '\tunsigned long kind= 1UL<<(header->type&0x1F);', '\tunsigned long kind= 1UL<<(header->type&0x20);'),
    (SOURCE_AI_ACTORS_C, '\treturn !(kind&3) || 1.0f>*(const real *)(object+0x2B0);', '\treturn !(kind&3) || 1.0f>*(const real *)(object+0x2b1);'),
    (SOURCE_AI_ACTORS_C, '\tlong unit_index= *(const long *)((const byte *)player_data->data+(player_index&0xFFFF)*0x21C+0x2C);', '\tlong unit_index= *(const long *)((const byte *)player_data->data+(player_index&0xFFFF)*0x21C+0x2d);'),
    (SOURCE_AI_ACTORS_C, '\treturn *(const long *)(unit+0x14)!=NONE && *(const short *)(unit+0x1FC)!=NONE;', '\treturn *(const long *)(unit+0x15)!=NONE && *(const short *)(unit+0x1FC)!=NONE;'),
    (SOURCE_AI_ACTORS_C, '\tif (unit_index==NONE)', '\tif (unit_index!=NONE)'),
    (SOURCE_AI_ACTORS_C, '\tunit= (const byte *)((const struct object_header_datum *)object_header_data->data)[unit_index&0xFFFF].datum;', ''),
    (SOURCE_AI_ACTORS_C, '\treturn (1UL<<(actor[0x86]&0x1F))|((1UL<<which)<<10);', '\treturn (1UL<<(actor[0x87]&0x1F))|((1UL<<which)<<10);'),
    (SOURCE_AI_ACTORS_C, '\t\twhich= 1;', ''),
    (SOURCE_AI_ACTORS_C, '\telse if (actor[0x266])', '\telse if (actor[0x267])'),
    (SOURCE_AI_ACTORS_C, '\tlong which= 0;', '\tlong which= 1;'),
]
