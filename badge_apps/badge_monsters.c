/*

This a got-a-collect-em-all style game that is meant to be played
with little PIC32 badges built by HackRVA for RVASec Conference
If the code seems a little strange, it's due to the environment this
code must run in.

--
Dustin Firebaugh <dafirebaugh@gmail.com>

*/

#ifdef __linux__
#include <string.h>

#include "../linux/linuxcompat.h"
#include <stdio.h>

#define DISABLE_INTERRUPTS do { disable_interrupts(); } while (0)
#define ENABLE_INTERRUPTS do { enable_interrupts(); } while (0)

#else

#include "colors.h"
#include "menu.h"
#include "buttons.h"
#include "flash.h"
#include "ir.h"

/* TODO: I shouldn't have to declare these myself. */
#define size_t int
extern char *strcpy(char *dest, const char *src);
extern char *strncpy(char *dest, const char *src, size_t n);
extern void *memset(void *s, int c, size_t n);
extern void *memcpy(void *dest, const void *src, size_t n);
extern char *strcat(char *dest, const char *src);
#ifndef NULL
#define NULL 0
#endif

#define DISABLE_INTERRUPTS
#define ENABLE_INTERRUPTS

#endif

#include "badge_monsters.h"

#define INIT_APP_STATE 0
#define GAME_MENU 1
#define RENDER_SCREEN 2
#define RENDER_MONSTER 3
#define TRADE_MONSTERS 4
#define CHECK_THE_BUTTONS 5
#define EXIT_APP 6

static void app_init(void);
static void game_menu(void);
static void menu_clear(void);
static void render_screen(void);
static void render_monster(void);
static void trade_monsters(void);
static void check_the_buttons(void);
static void setup_main_menu(void);
static void setup_monster_menu(void);
static void exit_app(void);

typedef void (*state_to_function_map_fn_type)(void);

static state_to_function_map_fn_type state_to_function_map[] = {
    app_init,
    game_menu,
    render_screen,
    render_monster,
    trade_monsters,
    check_the_buttons,
    exit_app,
};

#define TOTAL_BADGES 300
#define BADGE_ID G_sysData.badgeId
#define ARRAYSIZE(x) (sizeof((x)) / sizeof((x)[0]))
#define SCREEN_XDIM 132
#define SCREEN_YDIM 132


/* These need to be protected from interrupts. */
#define QUEUE_SIZE 5
static int queue_in;
static int queue_out;
static int packet_queue[QUEUE_SIZE] = { 0 };

static int screen_changed = 0;
static int smiley_x, smiley_y;
static int current_monster;
static int nmonsters = 0;
static int nvendor_monsters = 0;
static int app_state = INIT_APP_STATE;
static int trading_monsters_enabled = 0;

struct point
{
    signed char x, y;
};

static const struct point smiley_points[] =
#include "smileymon.h"
static const struct point freshmon_points[] =
#include "freshmon.h"
static const struct point mcturtle_points[] =
#include "mcturtle.h"
static const struct point goat_mon_points[] =
#include "goat_mon.h"
static const struct point hrvamon_points[] =
#include "hrvamon.h"
static const struct point octomon_points[] =
#include "octomon.h"
static const struct point zombieload_points[] =
#include "zombieload.h"
static const struct point spectre_points[] =
#include "spectre.h"
static const struct point heartbleed_points[] =
#include "heartbleed.h"
static const struct point stacksmasher_points[] =
#include "stacksmasher.h"
static const struct point worm_points[] =
#include "worm.h"
static const struct point godzilla_points[] =
#include "godzilla.h"
static const struct point eddie_points[] =
#include "eddie.h"

struct monster
{
    char name[20];
    int npoints;
    short status;
    int color;
    const struct point *drawing;
    char blurb[128];
};

struct monster monsters[] = {
    {"eddie", ARRAYSIZE(eddie_points), 0, WHITE, eddie_points, "Sometimes protective and Eddie is the perfect guy every girl wants to date"},
    {"godzilla", ARRAYSIZE(godzilla_points), 0, WHITE, godzilla_points, "a cross between a gorilla and a whale"},
    {"worm", ARRAYSIZE(worm_points), 0, WHITE, worm_points, "An earthworm is a tube-shaped, segmented worm found in the phylum Annelida."},
    {"stacksmasher", ARRAYSIZE(stacksmasher_points), 0, WHITE, stacksmasher_points, "Stack smashing is a form of vulnerability where the stack of a computer application or OS is forced to overflow."},
    {"heartbleed", ARRAYSIZE(heartbleed_points), 0, WHITE, heartbleed_points, "Heartbleed is a security bug in the OpenSSL cryptography library"},
    {"spectre", ARRAYSIZE(spectre_points), 0, WHITE, spectre_points, "Spectre (security vulnerability) Spectre is a vulnerability that affects modern microprocessors that perform branch prediction."},
    {"zombieload", ARRAYSIZE(zombieload_points), 0, WHITE, zombieload_points, "This one has the delightful name of ZombieLoad"},
    {"octomon", ARRAYSIZE(octomon_points), 0, WHITE, octomon_points, "a monster made with 8 sides"},
    {"hrvamon", ARRAYSIZE(hrvamon_points), 0, WHITE, hrvamon_points, "come check out hackrva maker space Thursday nights at 7. 1600 Roseneath Road Suite E Richmond, VA 23230"},
    {"mcturtle", ARRAYSIZE(mcturtle_points), 0, WHITE, mcturtle_points, "McTerrible McTurtle"},
    {"goat_mon", ARRAYSIZE(goat_mon_points), 0, WHITE, goat_mon_points, "this goat is scared because they are alone in the world"},
    {"freshmon", ARRAYSIZE(freshmon_points), 0, WHITE, freshmon_points, "a fusion of smirkmon and grinmon. This badge monster likes to troll people really hard for fun"},
    {"smileymon", ARRAYSIZE(smiley_points), 0, RED, smiley_points, "some nice words here"},
    {"smileymon", ARRAYSIZE(smiley_points), 0, WHITE, smiley_points, "Othermon some nice words here Othermon some nice words hereOthermon some nice words here Othermon some nice words here"},
};

struct monster vendor_monsters[] = {
    {"vgoatmon", ARRAYSIZE(smiley_points), 0, CYAN, goat_mon_points, "some nice words here"},
    {"vgoatmon", ARRAYSIZE(smiley_points), 0, CYAN, goat_mon_points, "some nice words here"},
    {"vsmileymon", ARRAYSIZE(smiley_points), 0, CYAN, smiley_points, "some nice words here"},
    {"vgoatmon", ARRAYSIZE(smiley_points), 0, CYAN, goat_mon_points, "some nice words here"}
};

int initial_mon;

#ifndef __linux__
/* Use draw_object() from maze.c */
extern void draw_object(const struct point drawing[], int npoints, int scale_index, int color, int x, int y);
#else
static void draw_object(const struct point drawing[], int npoints,
			__attribute__((unused)) int scale_index, int color, int x, int y)
{
	int i;
	int xcenter = x;
	int ycenter = y;
	int num;

	num = 410;	/* Yeah, ok, this is a bit too magic. Basically, it's */
			/* roughly 40% of 1024. the >> 10 below is where 1024 is */
			/* coming from.  Because 2^10 == 1024. So this is basically */
			/* scaling the drawing to 40% of its normal size. */

	FbColor(color);
	for (i = 0; i < npoints - 1;) {
		if (drawing[i].x == -128) {
			i++;
			continue;
		}
		if (drawing[i + 1].x == -128) {
			i+=2;
			continue;
		}
#if 0
		if (drawing[i].x < 0 || drawing[i].x > SCREEN_XDIM)
			continue;
		if (drawing[i].y < 0 || drawing[i].y > SCREEN_YDIM)
			continue;
#endif
		FbLine(xcenter + ((drawing[i].x * num) >> 10), ycenter + ((drawing[i].y * num) >> 10),
			xcenter + ((drawing[i + 1].x * num) >> 10), ycenter + ((drawing[i + 1].y * num) >> 10));
		i++;
	}
}
#endif

#ifndef __linux__
static void (*old_callback)(struct IRpacket_t) = NULL;
static void register_ir_packet_callback(void (*callback)(struct IRpacket_t))
{
	/* This is pretty gross.  Ideally there should be some registration,
	 * unregistration functions provided by ir.[ch] and I shouldn't touch
	 * IRcallbacks[] directly myself, and all those hardcoded ir_app[1-7]()
	 * functions should disappear.
	 * Also, if an interrupt happens in the midst of the assignment we'll
	 * be in trouble.  I expect the assignment is probably atomic though.
	 */
	old_callback = IRcallbacks[BADGE_IR_GAME_ADDRESS].handler;
	IRcallbacks[BADGE_IR_GAME_ADDRESS].handler = callback;
}

static void unregister_ir_packet_callback(void)
{
	/* Gross. */
	IRcallbacks[BADGE_IR_GAME_ADDRESS].handler = old_callback;
}
#endif

enum menu_level_t {
    MAIN_MENU,
    MONSTER_MENU,
    DESCRIPTION
} menu_level;

struct menu_item
{
    char text[15];
    state_to_function_map_fn_type next_state;
    unsigned char cookie;
};

static struct menu
{
    char title[15];
    struct menu_item item[20];
    unsigned char nitems;
    unsigned char current_item;
    unsigned char menu_active;
    unsigned char chosen_cookie;
} menu;

static void enable_monster(int monster_id)
{
    if(monster_id < 0 || monster_id > ARRAYSIZE(monsters) - 1)
        return;

    monsters[monster_id].status = 1;
    setNote(90, 4000);
    #ifdef __linux__
        printf("enabling monster: %d\n", monster_id);
    #endif
}

static unsigned int build_packet(unsigned char cmd, unsigned char start,
            unsigned char address, unsigned short badge_id, unsigned short payload)
{
    return ((cmd & 0x01) << 31) |
        ((start & 0x01) << 30) |
        ((address & 0x01f) << 25) |
        ((badge_id & 0x1ff) << 16) |
        (payload);
}

static unsigned short get_payload(unsigned int packet)
{
	return (unsigned short) (packet & 0x0ffff);
}

static void process_packet(unsigned int packet)
{
    unsigned int payload;
    unsigned char opcode;

    if (packet == 32) /* Ignore spurious 32 that might come in. */
        return;

    payload = get_payload(packet);
    opcode = payload >> 12;

    if(opcode == OPCODE_XMIT_MONSTER){
        enable_monster(payload & 0x0ff);
    }
}

static void send_ir_packet(unsigned int packet)
{
	union IRpacket_u p;

	p.v = packet;
	p.v = packet | (1 << 30); /* Set command bit on outgoing packets */

	IRqueueSend(p);
}

static void check_for_incoming_packets(void)
{
    unsigned int new_packet;
    int next_queue_out;
    DISABLE_INTERRUPTS;
    while (queue_out != queue_in) {
        next_queue_out = (queue_out + 1) % QUEUE_SIZE;
        new_packet = packet_queue[queue_out];
        queue_out = next_queue_out;
        ENABLE_INTERRUPTS;
        process_packet(new_packet);
        DISABLE_INTERRUPTS;
    }
    ENABLE_INTERRUPTS;
}

static void menu_clear(void)
{
    strncpy(menu.title, "", sizeof(menu.title) - 1);
    menu.nitems = 0;
    menu.current_item = 0;
    menu.chosen_cookie = 0;
}

static void menu_add_item(char *text, int next_state, unsigned char cookie)
{
    int i;

    if (menu.nitems >= ARRAYSIZE(menu.item))
        return;

    i = menu.nitems;
    strncpy(menu.item[i].text, text, sizeof(menu.item[i].text) - 1);
    menu.item[i].next_state = state_to_function_map[next_state];
    menu.item[i].cookie = cookie;
    menu.nitems++;
}

static void draw_menu(void)
{
    int i, y, first_item, last_item;

    first_item = menu.current_item - 4;
    if (first_item < 0)
        first_item = 0;
    last_item = menu.current_item + 4;
    if (last_item > menu.nitems - 1)
        last_item = menu.nitems - 1;

    FbClear();
    FbColor(WHITE);
    FbMove(8, 5);
    FbWriteLine(menu.title);
    if(menu_level != MONSTER_MENU){
        int nunlocked = 0;
        char available_monsters[3];
        char unlocked_monsters[3];
        for(i = 0; i < nmonsters; i++)
        {
            if(monsters[i].status == 1)
            {
                nunlocked++;
            }
        }

        for(i = 0; i < nvendor_monsters; i++)
        {
            if(vendor_monsters[i].status == 1)
            {
                nunlocked++;
            }
        }
        itoa(available_monsters, nmonsters + nvendor_monsters, 10);
        itoa(unlocked_monsters, nunlocked, 10);

        FbMove(1,25);
        FbWriteLine("Collected: ");
        FbWriteLine(unlocked_monsters);
        FbWriteLine("/");
        FbWriteLine(available_monsters);
    }

    y = SCREEN_YDIM / 2 - 12 * (menu.current_item - first_item);
    for (i = first_item; i <= last_item; i++)
    {
        if (i == menu.current_item)
            FbColor(GREEN);
        else
            FbColor(WHITE);
        FbMove(10, y);
        FbWriteLine(menu.item[i].text);
        y += 12;
    }

    FbColor(GREEN);
    FbHorizontalLine(5, SCREEN_YDIM / 2 - 2, SCREEN_XDIM - 5, SCREEN_YDIM / 2 - 2);
    FbHorizontalLine(5, SCREEN_YDIM / 2 + 10, SCREEN_XDIM - 5, SCREEN_YDIM / 2 + 10);
    FbVerticalLine(5, SCREEN_YDIM / 2 - 2, 5, SCREEN_YDIM / 2 + 10);
    FbVerticalLine(SCREEN_XDIM - 5, SCREEN_YDIM / 2 - 2, SCREEN_XDIM - 5, SCREEN_YDIM / 2 + 10);


    app_state = RENDER_SCREEN;
}

static void menu_change_current_selection(int direction)
{
    int old = menu.current_item;
    menu.current_item += direction;
    if (menu.current_item < 0)
        menu.current_item = menu.nitems - 1;
    else if (menu.current_item >= menu.nitems)
        menu.current_item = 0;
    screen_changed |= (menu.current_item != old);
}

static void change_menu_level(enum menu_level_t level)
{
    menu_level = level;
    menu_clear();
    switch(level){
        case MAIN_MENU:
            setup_main_menu();
            break;
        case MONSTER_MENU:
            setup_monster_menu();
            screen_changed = 1;
            break;
        case DESCRIPTION:
            return;
    }
}

static void show_message(char *message)
{
    #ifdef __linux__
        printf("%s\n", message);
    #endif

    FbClear();
    FbColor(WHITE);
    FbMove(8, 5);
    FbWriteLine(message);

    FbMove(5, 120);
    FbWriteLine("<Back");

    change_menu_level(DESCRIPTION);
    app_state = RENDER_SCREEN;
    screen_changed = 1;
}

static void render_monster(void)
{
    int npoints, color;
    const struct point *drawing = current_monster > 100 ? vendor_monsters[current_monster - 100].drawing : monsters[current_monster].drawing;
    char *name;

    if(current_monster > 100)
    {
        name = vendor_monsters[current_monster-100].name;
        npoints = vendor_monsters[current_monster-100].npoints;
        color = vendor_monsters[current_monster-100].color;
    }
    else
    {
        name = monsters[current_monster].name;
        npoints = monsters[current_monster].npoints;
        color = monsters[current_monster].color;
    }

    FbClear();
    if(current_monster == initial_mon)
    {
        FbMove(0,10);
        FbWriteLine("--starting-mon--");
    }
    FbMove(0,0);
    FbWriteLine(name);
    draw_object(drawing, npoints, 0, color, smiley_x, smiley_y);

    FbColor(WHITE);
    FbMove(43,120);
    FbWriteLine("|down|");
    FbColor(GREEN);

    FbMove(5, 120);
    FbWriteLine("<Back");


    FbMove(90,120);
    FbWriteLine("desc>");

    FbSwapBuffers();
    screen_changed = 1;
    app_state = RENDER_SCREEN;
}

static void trade_monsters(void)
{
	static counter = 0;

	counter++;
	if ((counter % 10000) == 0) { /* transmit our monster IR packet */
		send_ir_packet(build_packet(1,1,BADGE_IR_GAME_ADDRESS, BADGE_IR_BROADCAST_ID,
			(OPCODE_XMIT_MONSTER << 12) | (initial_mon & 0x01ff)));
	    setNote(50, 100);
	}
	if (!trading_monsters_enabled) {
		FbClear();
		FbMove(10, 60);
		FbWriteLine("TRADING");
		FbMove(10, 70);
		FbWriteLine("MONSTERS!");
		screen_changed = 1;
		trading_monsters_enabled = 1;
	}
	app_state = RENDER_SCREEN;
}

void render_screen_save_monsters(void) {
    static unsigned char current_index = 0;
    int npoints, color;
    if(current_index == ARRAYSIZE(monsters)) {
        current_index = 0;    
    }
    else {
        current_index++;
    }
    
    const struct point *drawing = monsters[current_index].drawing;
    npoints = monsters[current_index].npoints;
    color = monsters[current_index].color;
    FbClear();
    FbColor(BLACK);
    draw_object(drawing, npoints, 0, color, 64, 64);
}

static void render_screen(void)
{
    app_state = CHECK_THE_BUTTONS;
    if (!screen_changed)
        return;
    FbPushBuffer();
    screen_changed = 0;
}

#ifdef __linux__
static void print_menu_info(void)
{
    /* int next_state = menu.item[menu.current_item].next_state
       system("clear"); */
    printf("current item: %d\nmenu level: %d\ncurrent monster: %d\n menu item: %s\nn-menu-items: %d\ncookie_monster: %d\n",
    menu.current_item, menu_level, current_monster, menu.item[menu.current_item].text, menu.nitems, menu.item[menu.current_item].cookie);
}
#endif

static void check_the_buttons(void)
{
    int something_changed = 0;

    /* If we are trading monsters, stop trading monsters on a button press */
    if (trading_monsters_enabled) {
		if (UP_BTN_AND_CONSUME ||
			DOWN_BTN_AND_CONSUME ||
			LEFT_BTN_AND_CONSUME ||
			RIGHT_BTN_AND_CONSUME ||
			BUTTON_PRESSED_AND_CONSUME) {
			trading_monsters_enabled = 0;
			app_state = GAME_MENU;
			something_changed = 1;
			return;
		}
    }

    if (UP_BTN_AND_CONSUME)
    {
        switch(menu_level){
            case MAIN_MENU:
                menu_change_current_selection(-1);
                break;
            case MONSTER_MENU:
                menu_change_current_selection(-1);
                current_monster = menu.item[menu.current_item].cookie;
                render_monster();
                break;
            case DESCRIPTION:
                break;
        }
        something_changed = 1;
        #ifdef __linux__
            print_menu_info();
        #endif
    }
    else if (DOWN_BTN_AND_CONSUME)
    {
        switch(menu_level){
            case MAIN_MENU:
                menu_change_current_selection(1);
                break;
            case MONSTER_MENU:
                menu_change_current_selection(1);
                current_monster = menu.item[menu.current_item].cookie;
                render_monster();
                break;
            case DESCRIPTION:
                break;
        }
        something_changed = 1;
        #ifdef __linux__
            print_menu_info();
        #endif
    }
    else if (LEFT_BTN_AND_CONSUME)
    {
        switch(menu_level){
            case MAIN_MENU:
                break;
            case MONSTER_MENU:
                change_menu_level(MAIN_MENU);
                break;
            case DESCRIPTION:
                change_menu_level(MONSTER_MENU);
                break;
        }
        something_changed = 1;
    }
    else if (RIGHT_BTN_AND_CONSUME)
    {
        if(menu_level == MONSTER_MENU)
        {
            if(current_monster >= 100)
            {
                show_message(vendor_monsters[current_monster-100].blurb);
            }
            else
            {
                show_message(monsters[current_monster].blurb);
            }
        }
    }
    else if (BUTTON_PRESSED_AND_CONSUME)
    {
        int back = 0;
        if (menu_level == MONSTER_MENU)
        {
            if(menu.current_item == menu.nitems - 1){
                back = 1;
                something_changed = 1;
            } else {
                something_changed = 1;
                app_state = RENDER_MONSTER;
            }
        }

        if (menu_level == MAIN_MENU)
        {
            switch(menu.current_item){
                case 0:
                    change_menu_level(MONSTER_MENU);
                    current_monster = menu.item[menu.current_item].cookie;
                    something_changed = 1;
                    break;
                case 1:
		    app_state = TRADE_MONSTERS;
		    something_changed = 1;
                    break;
                case 2:
                    app_state = EXIT_APP;
                    break;
            }
        }

        /* if the back button is pressed we will return to the main menu */
        if(back || menu_level == DESCRIPTION)
            change_menu_level(MAIN_MENU);
    }
    if (trading_monsters_enabled && !something_changed) {
	app_state = TRADE_MONSTERS;
	return;
    }

    if (something_changed && app_state == CHECK_THE_BUTTONS)
        app_state = GAME_MENU;
    return;
}

static void setup_monster_menu(void)
{
    int i;
    menu_clear();
    menu.menu_active = 0;
    strcpy(menu.title, "Monsters");

    for(i = 0; i < nmonsters; i++){
        if(monsters[i].status)
            menu_add_item(monsters[i].name, RENDER_MONSTER, i);
    }

    #if 0
    for(i = 0; i < nvendor_monsters; i++){
        if(vendor_monsters[i].status)
            menu_add_item(vendor_monsters[i].name, RENDER_MONSTER, i+100);
    }
    #endif

    screen_changed = 1;
    render_monster();
}

static void setup_main_menu(void)
{
    menu_clear();
    strcpy(menu.title, "Badge Monsters");
    menu_add_item("Monsters", RENDER_SCREEN, 0);
    menu_add_item("Trade Monsters", TRADE_MONSTERS, 1);
    menu_add_item("EXIT", EXIT_APP, 2);
    screen_changed = 1;
}

static void game_menu(void)
{
    draw_menu();
    check_for_incoming_packets();
    app_state = RENDER_SCREEN;
}

static void load_from_flash(void){
    /*
    load from flash should load a list of monsterIDs that have been enabled
    for each monsterID, it should run enable_monster() function
    */
   #if 0
   int i;
   for(i = 0; i < ARRAYSIZE(monstersIDs); i++)
      enable_monster(monsterID);
   #endif
}

static void save_to_flash(void){
    /*
    save to flash should save some form of a list of monsterIDs that have been unlocked
    */
}

static void exit_app(void)
{
    app_state = INIT_APP_STATE;
    save_to_flash();
    unregister_ir_packet_callback();
    returnToMenus();
}

static void ir_packet_callback(struct IRpacket_t packet)
{
	/* Interrupts will be already disabled when this is called. */
	int next_queue_in;

	next_queue_in = (queue_in + 1) % QUEUE_SIZE;
	if (next_queue_in == queue_out) /* queue is full, drop packet */
		return;
	memcpy(&packet_queue[queue_in], &packet, sizeof(packet_queue[0]));
	queue_in = next_queue_in;
}

static void app_init(void)
{
    int i;

    FbInit();
    app_state = INIT_APP_STATE;
    register_ir_packet_callback(ir_packet_callback);

    change_menu_level(MAIN_MENU);
    app_state = GAME_MENU;
    screen_changed = 1;
    smiley_x = SCREEN_XDIM / 2;
    smiley_y = SCREEN_XDIM / 2;
    nmonsters = ARRAYSIZE(monsters);
    nvendor_monsters = ARRAYSIZE(vendor_monsters);
    initial_mon = BADGE_ID % nmonsters;
    current_monster = initial_mon;
    enable_monster(initial_mon);
    load_from_flash();

    for (i = 0; i < ARRAYSIZE(monsters); i++)
	    enable_monster(i);
}

int badge_monsters_cb(void)
{
    state_to_function_map[app_state]();
    return 0;
}

#ifdef __linux__

int main(int argc, char *argv[])
{
    char *serial_port = NULL;

#define IRXMIT_UDP_PORT 12345
#define LASERTAG_UDP_PORT 12346

	if (argc >= 2)
		serial_port = argv[1];
	setup_linux_ir_simulator(serial_port, IRXMIT_UDP_PORT, LASERTAG_UDP_PORT);
    start_gtk(&argc, &argv, badge_monsters_cb, 30);
    return 0;
}

#endif
