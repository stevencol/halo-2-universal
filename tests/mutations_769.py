# mutations of milestone 769: one or more lines of each function

TESTS = "test_small_769"

LISTS = "source/interface/user_interface_lists.c"

MUTATIONS = [
    (LISTS, "\treturn value>=0 && value<=USER_INTERFACE_UNKNOWN4_MAXIMUM;", "\treturn value>=0 && value<USER_INTERFACE_UNKNOWN4_MAXIMUM;"),
    (LISTS, "\tif (!other)\n\t{\n\t\treturn;", "\tif (FALSE)\n\t{\n\t\treturn;"),
    (LISTS, "\tif (item==LINK_AT(list, USER_INTERFACE_LIST_FIRST_OFFSET))\n\t{", "\tif (FALSE)\n\t{"),
    (LISTS, "\t\tLINK_AT(next, USER_INTERFACE_LIST_ITEM_PREVIOUS_OFFSET)= LINK_AT(item, USER_INTERFACE_LIST_ITEM_PREVIOUS_OFFSET);", "\t\tLINK_AT(next, USER_INTERFACE_LIST_ITEM_PREVIOUS_OFFSET)= NULL;"),
    (LISTS, "\t*(long *)(item+USER_INTERFACE_LIST_ITEM_INDEX_OFFSET)= NONE;", "\t*(long *)(item+USER_INTERFACE_LIST_ITEM_INDEX_OFFSET)= 0;"),
    (LISTS, "\t\twhile (LINK_AT(child, USER_INTERFACE_TREE_FIRST_CHILD_OFFSET))", "\t\tif (LINK_AT(child, USER_INTERFACE_TREE_FIRST_CHILD_OFFSET))"),
    (LISTS, "\treturn *(const long *)(thing+USER_INTERFACE_TREE_UNKNOWN4_OFFSET) ? NULL : thing;", "\treturn *(const long *)(thing+USER_INTERFACE_TREE_UNKNOWN4_OFFSET) ? thing : NULL;"),
    (LISTS, "\tlink->list= list;", "\tlink->list= NULL;"),
    (LISTS, "\tlink->previous= last;", "\tlink->previous= NULL;"),
    (LISTS, "\t\tlink->next->previous= link->previous;", "\t\tlink->next->previous= link;"),
    (LISTS, "\tif (*list==link)\n\t{\n\t\t*list= next;", "\tif (*list==link)\n\t{\n\t\t*list= link->next;"),
    (LISTS, "\treturn ((flags>>1)&1) || (flags&1);", "\treturn (flags>>1)&1;"),
]
