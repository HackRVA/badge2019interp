#include "menu.h"

#ifndef NULL
#define NULL 0
#endif

const struct menu_t breakfast_m[] = {
//   {"yummy", VERT_ITEM|DEFAULT_ITEM, FUNCTION, {(struct menu_t *)yummy_unlock_1}}, /* can init union either with or without {} */
   {"back", VERT_ITEM|LAST_ITEM, BACK, {NULL}},
};


const struct menu_t day2_p3_m[] = {
   {" 2:00 101Panel", VERT_ITEM, TEXT, {NULL}},
   {" 2:00 CodeSign", VERT_ITEM, TEXT, {NULL}},
   {" 2:00 ATT&CK", VERT_ITEM, TEXT, {NULL}},
   {" 2:50 -Break-", VERT_ITEM, TEXT, {NULL}},
   {" 3:00 CISO2025", VERT_ITEM, TEXT, {NULL}},
   {" 3:50 Closing", VERT_ITEM, TEXT, {NULL}},
   {" 4:00 Rec. Awrds", VERT_ITEM, TEXT, {NULL}},
   {"back", VERT_ITEM|LAST_ITEM, BACK, {NULL}},
};

const struct menu_t day2_p2_m[] = {
   {"11:10 NetSec101", VERT_ITEM, TEXT, {NULL}},
   {"11:10 B and E", VERT_ITEM, TEXT, {NULL}},
   {"11:10 CSRF", VERT_ITEM, TEXT, {NULL}},
   {"12:00 Lunch", VERT_ITEM, TEXT, {NULL}},
   {" 1:00 Mng Risk", VERT_ITEM, TEXT, {NULL}},
   {" 1:00 Mind Exp", VERT_ITEM, TEXT, {NULL}},
   {" 1:00 NetSecMon", VERT_ITEM, TEXT, {NULL}},
   {" 1:50 -break-", VERT_ITEM, TEXT, {NULL}},
   {"back", VERT_ITEM|LAST_ITEM, BACK, {NULL}},
};

const struct menu_t day2_p1_m[] = {
   {" 8:00 Breakfast", VERT_ITEM, MENU, {breakfast_m}}, // breakfast easter egg
   {" 8:00 Registrat", VERT_ITEM, TEXT, {NULL}},
   {" 8:50 Welcome", VERT_ITEM, TEXT, {NULL}},
   {" 9:00 Keynote", VERT_ITEM, TEXT, {NULL}},
   {"10:00 -break-", VERT_ITEM, TEXT, {NULL}},
   {"10:10 Risk Assmt", VERT_ITEM, TEXT, {NULL}},
   {"10:10 Mix Sec Bk", VERT_ITEM, TEXT, {NULL}},
   {"10:10 Zero2Hero", VERT_ITEM, TEXT, {NULL}},
   {"10:10 CTF", VERT_ITEM, TEXT, {NULL}},
   {"11:00 -break-", VERT_ITEM, TEXT, {NULL}},
   {"back", VERT_ITEM|LAST_ITEM, BACK, {NULL}},
};


const struct menu_t day1_p3_m[] = {
   {" 2:00 Vuln Asses", VERT_ITEM, TEXT, {NULL}},
   {" 2:00 Sec 9-1-1", VERT_ITEM, TEXT, {NULL}},
   {" 2:00 DHCP DNS", VERT_ITEM, TEXT, {NULL}},
   {" 2:50 -Break-", VERT_ITEM, TEXT, {NULL}},
   {" 3:00 Soc Eng", VERT_ITEM, TEXT, {NULL}},
   {" 3:00 Compliance", VERT_ITEM, TEXT, {NULL}},
   {" 3:00 Trenches", VERT_ITEM, TEXT, {NULL}},
   {" 3:50 -Break-", VERT_ITEM, TEXT, {NULL}},
   {" 4:00 No Whackam", VERT_ITEM, TEXT, {NULL}},
   {" 4:50 Closing", VERT_ITEM, TEXT, {NULL}},
   {"back", VERT_ITEM|LAST_ITEM, BACK, {NULL}},
};


const struct menu_t day1_p2_m[] = {
   {"11:00 Intro 101", VERT_ITEM, TEXT, {NULL}},
   {"11:00 Automating", VERT_ITEM, TEXT, {NULL}},
   {"11:00 Malic File", VERT_ITEM, TEXT, {NULL}},
   {"11:50 Lunch", VERT_ITEM, TEXT, {NULL}},
   {" 1:00 Cyber Ins.", VERT_ITEM, TEXT, {NULL}},
   {" 1:00 1 Man Army", VERT_ITEM, TEXT, {NULL}},
   {" 1:00 Govt Red T", VERT_ITEM, TEXT, {NULL}},
   {" 1:00 CTF Prep", VERT_ITEM, TEXT, {NULL}},
   {" 1:50 -Break-", VERT_ITEM, TEXT, {NULL}},
   {"back", VERT_ITEM|LAST_ITEM, BACK, {NULL}},
};


const struct menu_t day1_p1_m[] = {
   {" 8:00 Breakfast", VERT_ITEM, MENU, {breakfast_m}},
   {" 8:00 Registrat", VERT_ITEM, TEXT, {NULL}},
   {" 9:00 Welcome", VERT_ITEM, TEXT, {NULL}},
   {" 9:10 Keynote", VERT_ITEM, TEXT, {NULL}},
   {"10:10 -Break-", VERT_ITEM, TEXT, {NULL}},
   {"10:30 CTF Intro", VERT_ITEM, TEXT, {NULL}},
   {"10:40 Badge", VERT_ITEM, TEXT, {NULL}},
   {"10:50 -Break-", VERT_ITEM, TEXT, {NULL}},
   {"back", VERT_ITEM|LAST_ITEM, BACK, {NULL}},
};


const struct menu_t schedule_m[] = {
   {"Wednesday", VERT_ITEM|SKIP_ITEM, TEXT, {NULL}},
   {"  8-10:50", VERT_ITEM|DEFAULT_ITEM, MENU, {day1_p1_m}},
   {"  11-1:50", VERT_ITEM, MENU, {day1_p2_m}},
   {"  2-4:50", VERT_ITEM, MENU, {day1_p3_m}},
   {"  5:30 Aft Prty", VERT_ITEM|SKIP_ITEM, TEXT, {NULL}},
   {"Thursday", VERT_ITEM|SKIP_ITEM, TEXT, {NULL}},
   {"  8-11:00", VERT_ITEM, MENU, {day2_p1_m}},
   {"  11:10-1:50", VERT_ITEM, MENU, {day2_p2_m}},
   {"  2-Close", VERT_ITEM, MENU, {day2_p3_m}},
   {"back", VERT_ITEM|LAST_ITEM, BACK, {NULL}},
};

