const struct menu_t breakfast_m[] = {
   {"yummy", VERT_ITEM|DEFAULT_ITEM, FUNCTION, {(struct menu_t *)yummy_unlock_1}}, /* can init union either with or without {} */
   {"back", VERT_ITEM|LAST_ITEM, BACK, {NULL}},
};


const struct menu_t day2_salons_p1_m[] = {
   {"10:10 Mark P.", VERT_ITEM, TEXT, {NULL}},
   {"11:00 -Break-", VERT_ITEM, TEXT, {NULL}},
   {"11:10 M.S. T.W", VERT_ITEM, TEXT, {NULL}},
   {"12:00 Lunch", VERT_ITEM, TEXT, {NULL}},
   {" 1:50 -Break-", VERT_ITEM, TEXT, {NULL}},
   {" 2:00 Elissa", VERT_ITEM, TEXT, {NULL}},
   {"back", VERT_ITEM, BACK, {NULL}},
   {"exit", VERT_ITEM|DEFAULT_ITEM|LAST_ITEM,FUNCTION, {(struct menu_t *)to_main_menu_cb}},
};

const struct menu_t day1_salons_p2_m[] = {
   {" 3:50 -Break-", VERT_ITEM, TEXT, {NULL}},
   {" 4:00 Robert S.", VERT_ITEM, TEXT, {NULL}},
   {" 4:50 -Break-", VERT_ITEM, TEXT, {NULL}},
   {"back", VERT_ITEM, BACK, {NULL}},
   {"exit", VERT_ITEM|DEFAULT_ITEM|LAST_ITEM,FUNCTION, {(struct menu_t *)to_main_menu_cb}},
};

const struct menu_t day1_salons_p1_m[] = {
   {"11:00 Barry K.", VERT_ITEM, TEXT, {NULL}},
   {"11:50 Lunch", VERT_ITEM, TEXT, {NULL}},
   {" 1:00 Allen H.", VERT_ITEM, TEXT, {NULL}},
   {" 1:50 -Break-", VERT_ITEM, TEXT, {NULL}},
   {" 2:00 Schuyler", VERT_ITEM, TEXT, {NULL}},
   {" 2:50 -Break-", VERT_ITEM, TEXT, {NULL}},
   {" 3:00 Adam C.", VERT_ITEM, TEXT, {NULL}},
   {"Next", VERT_ITEM|DEFAULT_ITEM, MENU, {day1_salons_p2_m}},
   {"back", VERT_ITEM, BACK, {NULL}},
   {"exit", VERT_ITEM|LAST_ITEM,FUNCTION, {(struct menu_t *)to_main_menu_cb}},
};

const struct menu_t day2_ballroom_p2_m[] = {
   {"12:00 Lunch", VERT_ITEM, TEXT, {NULL}},
   {" 1:00 G-Man", VERT_ITEM, TEXT, {NULL}},
   {" 1:50 -Break-", VERT_ITEM, TEXT, {NULL}},
   {" 2:00 Borris S.", VERT_ITEM, TEXT, {NULL}},
   {" 2:50 -Break-", VERT_ITEM, TEXT, {NULL}},
   {" 3:00 Debates", VERT_ITEM, TEXT, {NULL}},
   {" 4:00 Closing", VERT_ITEM, TEXT, {NULL}},
   {"back", VERT_ITEM, BACK, {NULL}},
   {"exit", VERT_ITEM|DEFAULT_ITEM|LAST_ITEM,FUNCTION, {(struct menu_t *)to_main_menu_cb}},
};

const struct menu_t day2_ballroom_p1_m[] = {
   {" 8:00 Breakfast", VERT_ITEM, MENU, {breakfast_m}}, // breakfast easter egg
   {" 8:50 Welcome", VERT_ITEM, TEXT, {NULL}},
   {" 9:00 Keynote", VERT_ITEM, TEXT, {NULL}},
   {"10:00 -Break-", VERT_ITEM, TEXT, {NULL}},
   {"10:10 David B.", VERT_ITEM, TEXT, {NULL}},
   {"11:00 -Break-", VERT_ITEM, TEXT, {NULL}},
   {"11:10 Bill W.", VERT_ITEM, TEXT, {NULL}},
   {"Next", VERT_ITEM|DEFAULT_ITEM, MENU, {day2_ballroom_p2_m}},
   {"back", VERT_ITEM, BACK, {NULL}},
   {"exit", VERT_ITEM|LAST_ITEM,FUNCTION, {(struct menu_t *)to_main_menu_cb}},
};


const struct menu_t day1_ballroom_p3_m[] = {
   {" 4:00 David L.", VERT_ITEM, MENU, {NULL}},
   {" 4:50 -Break-", VERT_ITEM, MENU, {NULL}},
   {" 5:00 Jason S.", VERT_ITEM, MENU, {NULL}},
   {" 5:50 -Closing-", VERT_ITEM, MENU, {NULL}},
   {" 6:30 !Party!", VERT_ITEM, MENU, {NULL}},
   {"back", VERT_ITEM, BACK, {NULL}},
   {"exit", VERT_ITEM|DEFAULT_ITEM|LAST_ITEM,FUNCTION, {(struct menu_t *)to_main_menu_cb}},
};


const struct menu_t day1_ballroom_p2_m[] = {
   {"11:50 -lunch-", VERT_ITEM, TEXT, {NULL}},
   {" 1:00 kizz m.", VERT_ITEM, TEXT, {NULL}},
   {" 1:50 -break-", VERT_ITEM, TEXT, {NULL}},
   {" 2:00 chill", VERT_ITEM, TEXT, {NULL}},
   {" 2:50 -break-", VERT_ITEM, TEXT, {NULL}},
   {" 3:00 P.H. D", VERT_ITEM, TEXT, {NULL}},
   {" 3:50 -break-", VERT_ITEM, TEXT, {NULL}},
   {"next", VERT_ITEM|DEFAULT_ITEM, MENU, {day1_ballroom_p3_m}},
   {"back", VERT_ITEM, BACK, {NULL}},
   {"exit", VERT_ITEM|LAST_ITEM,FUNCTION, {(struct menu_t *)to_main_menu_cb}},
};


const struct menu_t day1_ballroom_p1_m[] = {
   {" 8:00 Breakfast", VERT_ITEM, MENU, {breakfast_m}},
   {" 9:00 Welcome", VERT_ITEM, TEXT, {NULL}},
   {" 9:10 Keynote", VERT_ITEM, TEXT, {NULL}},
   {"10:10 CTF Intro", VERT_ITEM, TEXT, {NULL}},
   {"10:20 Badges!", VERT_ITEM, TEXT, {NULL}},
   {"10:30 -Break-", VERT_ITEM, TEXT, {NULL}},
   {"11:00 Jason S.", VERT_ITEM, TEXT, {NULL}},
   {"Next", VERT_ITEM|DEFAULT_ITEM, MENU, {day1_ballroom_p2_m}},
   {"exit", VERT_ITEM|LAST_ITEM,FUNCTION, {(struct menu_t *)to_main_menu_cb}},
};


const struct menu_t schedule_m[] = {
    {"-Ballroom-", VERT_ITEM|SKIP_ITEM, TEXT, {NULL}},
   {"  Day 1", VERT_ITEM|DEFAULT_ITEM, MENU, {day1_ballroom_p1_m}},
   {"  Day 2", VERT_ITEM, MENU, {day2_ballroom_p1_m}},
   {"", VERT_ITEM|SKIP_ITEM, TEXT, {NULL}},
   {"-Salons-", VERT_ITEM|SKIP_ITEM, TEXT, {NULL}},
   {"  Day 1", VERT_ITEM, MENU, {day1_salons_p1_m}},
   {"  Day 2", VERT_ITEM, MENU, {day2_salons_p1_m}},
   {"", VERT_ITEM|SKIP_ITEM, TEXT, {NULL}},
   {"", VERT_ITEM|SKIP_ITEM, TEXT, {NULL}},
   {"Exit", VERT_ITEM|LAST_ITEM, BACK, {NULL}},
};

