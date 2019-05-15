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
extern char *strcat(char *dest, const char *src);

#define DISABLE_INTERRUPTS
#define ENABLE_INTERRUPTS

#endif

#define INIT_APP_STATE 0
#define GAME_MENU 1
#define RENDER_SCREEN 2
#define RENDER_MONSTER 3
#define CHECK_THE_BUTTONS 4
#define EXIT_APP 5

static void app_init(void);
static void game_menu(void);
static void menu_clear(void);
static void render_screen(void);
static void render_monster(void);
static void check_the_buttons(void);
static void setup_monster_menu(void);
static void setup_main_menu(void);
static void exit_app(void);

typedef void (*state_to_function_map_fn_type)(void);

static state_to_function_map_fn_type state_to_function_map[] = {
    app_init,
    game_menu,
    render_screen,
    render_monster,
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
// static int queue_in;
// static int queue_out;
// static int packet_queue[QUEUE_SIZE] = { 0 };

static int screen_changed = 0;
static int smiley_x, smiley_y;
static int current_monster = 0;
static int nmonsters = 0;
static int nvendor_monsters = 0;
static int app_state = INIT_APP_STATE;

static struct point
{
    signed char x, y;
};

static struct point smiley_points[] =
#include "smileymon.h"

static struct point othermon_points[] =
#include "othermon.h"

static struct monster
{
    char name[20];
    int npoints;
    short status;
    int color;
    struct point *drawing;
    char blurb[128];
};

struct monster monsters[] = {
    {"othermon", ARRAYSIZE(smiley_points), 0, 0, othermon_points, "some nice words here"},
    {"othermon", ARRAYSIZE(smiley_points), 0, 1, othermon_points, "some nice words here"},
    {"smileymon", ARRAYSIZE(smiley_points), 1, RED, smiley_points, "some nice words here"},
    {"othermon", ARRAYSIZE(smiley_points), 0, 0, othermon_points, "some nice words here"},
    {"othermon", ARRAYSIZE(smiley_points), 0, 1, othermon_points, "some nice words here"},
    {"smileymon", ARRAYSIZE(smiley_points), 1, WHITE, smiley_points, "Othermon some nice words here Othermon some nice words hereOthermon some nice words here Othermon some nice words here"},
    {"othermon", ARRAYSIZE(smiley_points), 0, 0, othermon_points, "some nice words here"},
    {"othermon", ARRAYSIZE(smiley_points), 0, WHITE, othermon_points, "some nice words here"},
    {"othermon", ARRAYSIZE(smiley_points), 0, WHITE, othermon_points, "some nice words here"},
};

struct monster vendor_monsters[] = {
    {"vothermon", ARRAYSIZE(smiley_points), 0, CYAN, othermon_points, "some nice words here"},
    {"vothermon", ARRAYSIZE(smiley_points), 0, CYAN, othermon_points, "some nice words here"},
    {"vsmileymon", ARRAYSIZE(smiley_points), 1, CYAN, smiley_points, "some nice words here"},
    {"vothermon", ARRAYSIZE(smiley_points), 0, CYAN, othermon_points, "some nice words here"}
};

static void draw_object(struct point drawing[], int npoints, int color, int x, int y)
{
    int i;
    int xcenter = x;
    int ycenter = y;
    int num;

    num = 410; /* Yeah, ok, this is a bit too magic. Basically, it's */
               /* roughly 40% of 1024. the >> 10 below is where 1024 is */
               /* coming from.  Because 2^10 == 1024. So this is basically */
               /* scaling the drawing to 40% of its normal size. */

    FbColor(color);
    for (i = 0; i < npoints - 1;)
    {
        if (drawing[i].x == -128)
        {
            i++;
            continue;
        }
        if (drawing[i + 1].x == -128)
        {
            i += 2;
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

enum menu_level_t {
    MAIN_MENU,
    MONSTER_MENU,
    INACTIVE
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

// static void process_packet(unsigned int packet)
// {
//     unsigned int payload;
//     unsigned char opcode;
//     int v;

//     if (packet == 32) /* Ignore spurious 32 that might come in. */
//         return;

//     payload = get_payload(packet);
//     opcode = payload >> 12;
//
//     // switch (opcode) {
//     // case OPCODE_SET_GAME_START_TIME:
//     //     /* time is a 12 bit signed number */
//     //     v = payload & 0x0fff;
//     //     if (payload & 0x0800)
//     //         v = -v;
//     //     seconds_until_game_starts = v;
//     //     set_game_start_timestamp(seconds_until_game_starts);
//     //     if (seconds_until_game_starts > 0)
//     //         nhits = 0; /* don't reset counter if game already started? */
//     //     screen_changed = 1;
//     //     break;
//     // case OPCODE_SET_GAME_DURATION:
//     //     /* time is 12 unsigned number */
//     //     game_duration = payload & 0x0fff;
//     //     screen_changed = 1;
//     //     break;
//     // case OPCODE_HIT:
//     //     process_hit(packet);
//     //     break;
//     // case OPCODE_REQUEST_BADGE_DUMP:
//     //     game_state = GAME_DUMP_DATA;
//     //     break;
//     // case OPCODE_SET_BADGE_TEAM:
//     //     team = payload & 0x0f; /* TODO sanity check this better. */
//     //     screen_changed = 1;
//     //     break;
//     // case OPCODE_SET_GAME_VARIANT:
//     //     game_variant = (payload & 0x0f) % ARRAYSIZE(game_type);
//     //     screen_changed = 1;
//     //     break;
//     // case OPCODE_GAME_ID:
//     //     game_id = payload & 0x0fff;
//         /* We happen to know this is the last bit of data for a game that the base
//          * station sends us. So at this time, we beep to indicate all the data for
//          * the game has been recieved. */
//     //     setNote(50, 4000);
//     //     break;
//     // default:
//     //     break;
//     // }
// }

// static unsigned int build_packet(unsigned char cmd, unsigned char start,
//             unsigned char address, unsigned short badge_id, unsigned short payload)
// {
//     return ((cmd & 0x01) << 31) |
//         ((start & 0x01) << 30) |
//         ((address & 0x01f) << 25) |
//         ((badge_id & 0x1ff) << 16) |
//         (payload);
// }

// static void send_a_packet(unsigned int packet)
// {
//     union IRpacket_u p;

//     p.v = packet;
//     IRqueueSend(p);

// #ifdef __linux__
//     printf("\nSent packet: %08x\n", packet);
//     printf("      cmd: 0x%01x\n", (packet >> 31) & 0x01);
//     printf("    start: 0x%01x\n", (packet >> 30) & 0x01);
//     printf("  address: 0x%02x\n", (packet >> 25) & 0x1f);
//     printf(" badge ID: 0x%03x\n", (packet >> 16) & 0x1ff);
//     printf("  payload: 0x%04x\n\n", packet & 0x0ffff);
// #endif

// }

// static void check_for_incoming_packets(void)
// {
//     unsigned int new_packet;
//     int next_queue_out;

//     DISABLE_INTERRUPTS;
//     while (queue_out != queue_in) {
//         next_queue_out = (queue_out + 1) % QUEUE_SIZE;
//         new_packet = packet_queue[queue_out];
//         queue_out = next_queue_out;
//         ENABLE_INTERRUPTS;
//         // process_packet(new_packet);
//         DISABLE_INTERRUPTS;
//     }
//     ENABLE_INTERRUPTS;
// }

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
    if(menu_level == MONSTER_MENU){
        int nunlocked = 0;
        char available_monsters[3];
        char unlocked_monsters[3];
        itoa(available_monsters, nmonsters + nvendor_monsters, 5);

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



        itoa(unlocked_monsters, nunlocked, 5);

        FbMove(8,25);
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

static void change_menu_level(enum menu_level_t level){
    menu_level = level;
    menu_clear();
    switch(level){
        case MAIN_MENU:
            setup_main_menu();
            break;
        case MONSTER_MENU:
            setup_monster_menu();
            break;
        case INACTIVE:
            return;
    }
}

static void enable_monster(int monster_id){
    monsters[monster_id].status = 1;
    #ifdef __linux__
        printf("enabling monster: %d\n", monster_id);
    #endif
}

static void show_message(char *message){
    #ifdef __linux__
        printf(message);
    #endif

    FbClear();
    FbColor(WHITE);
    FbMove(8, 5);
    FbWriteLine(message);

    change_menu_level(INACTIVE);
    app_state = RENDER_SCREEN;
    screen_changed = 1;
}

// stage_monster_trade -- should start listening and receiving IR
static void stage_monster_trade(void){


    show_message("Sync your badge with someone to collect more     monsters\n");
}

static void render_monster(void)
{
    int npoints, color;
    struct point *drawing;
    char *name;

    if(current_monster > 100)
    {
        printf("um %d\n",current_monster);
        name = vendor_monsters[current_monster-100].name;
        drawing = vendor_monsters[current_monster-100].drawing;
        npoints = vendor_monsters[current_monster-100].npoints;
        color = vendor_monsters[current_monster-100].color;
    }
    else
    {
        name = monsters[current_monster].name;
        drawing = monsters[current_monster].drawing;
        npoints = monsters[current_monster].npoints;
        color = monsters[current_monster].color;
    }


    FbClear();
    FbWriteLine(name);
    FbWriteLine("\n");
    draw_object(drawing, npoints, color, smiley_x, smiley_y);

    FbMove(120,120);
    FbWriteLine(">");
    FbSwapBuffers();
    change_menu_level(INACTIVE);
    screen_changed = 1;
    app_state = RENDER_SCREEN;
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
static void print_menu_info(void){
    // int next_state = menu.item[menu.current_item].next_state
    system("clear");
    printf("current item: %d\nmenu level: %d\ncurrent monster: %d\n menu item: %s\nn-menu-items: %d\ncookie_monster: %d\n",
    menu.current_item, menu_level, current_monster, menu.item[menu.current_item].text, menu.nitems, menu.item[menu.current_item].cookie);
}
#endif

static void check_the_buttons(void)
{

    int something_changed = 0;

    if (UP_BTN_AND_CONSUME)
    {
        if(menu_level == INACTIVE)
            change_menu_level(MONSTER_MENU);
        something_changed = 1;
        menu_change_current_selection(-1);
        if(menu_level == MONSTER_MENU)
            current_monster = menu.item[menu.current_item].cookie;
        #ifdef __linux__
            print_menu_info();
        #endif
    }
    else if (DOWN_BTN_AND_CONSUME)
    {
        if(menu_level == INACTIVE)
            change_menu_level(MONSTER_MENU);
        something_changed = 1;
        menu_change_current_selection(1);
        if(menu_level == MONSTER_MENU)
            current_monster = menu.item[menu.current_item].cookie;
        #ifdef __linux__
            print_menu_info();
        #endif
    }
    else if (LEFT_BTN_AND_CONSUME)
    {
        if(menu_level == INACTIVE)
            change_menu_level(MONSTER_MENU);
        something_changed = 1;
    }
    else if (RIGHT_BTN_AND_CONSUME)
    {
        if(menu_level == INACTIVE)
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
                    stage_monster_trade();
                    break;
                case 2:
                    app_state = EXIT_APP;
                    break;
            }
        }

        // if the back button is pressed we will return to the main menu
        if(back || menu_level == INACTIVE)
            change_menu_level(MAIN_MENU);
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
    current_monster = 0;

    for(i = 0; i < nmonsters; i++){
        if(monsters[i].status)
            menu_add_item(monsters[i].name, RENDER_MONSTER, i);
    }

    for(i = 0; i < nvendor_monsters; i++){
        if(vendor_monsters[i].status)
            menu_add_item(vendor_monsters[i].name, RENDER_MONSTER, i+100);
    }

    menu_add_item("back", RENDER_SCREEN, 0);
    screen_changed = 1;
}

static void setup_main_menu(void)
{
    menu_clear();
    strcpy(menu.title, "Badge Monsters");
    menu_add_item("Monsters", RENDER_SCREEN, 0);
    menu_add_item("Trade Monsters", RENDER_SCREEN, 1);
    menu_add_item("EXIT", EXIT_APP, 2);
    screen_changed = 1;
}

static void game_menu(void)
{
    draw_menu();
    app_state = RENDER_SCREEN;
}

static void exit_app(void)
{
    app_state = INIT_APP_STATE;
    returnToMenus();
}

static void app_init(void)
{
    FbInit();
    app_state = INIT_APP_STATE;
    change_menu_level(MAIN_MENU);
    app_state = GAME_MENU;
    screen_changed = 1;
    smiley_x = SCREEN_XDIM / 2;
    smiley_y = SCREEN_XDIM / 2;
    nmonsters = ARRAYSIZE(monsters);
    nvendor_monsters = ARRAYSIZE(vendor_monsters);
    int initial_mon = BADGE_ID % nmonsters;
    enable_monster(initial_mon);
}

int badge_monsters_cb(void)
{
    state_to_function_map[app_state]();
    return 0;
}

#ifdef __linux__

int main(int argc, char *argv[])
{
    start_gtk(&argc, &argv, badge_monsters_cb, 30);
    return 0;
}

#endif
