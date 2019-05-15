#ifdef __linux__
#include <string.h>

#include "../linux/linuxcompat.h"
#include <stdio.h>

#else

#include "colors.h"
#include "menu.h"
#include "buttons.h"

/* TODO: I shouldn't have to declare these myself. */
#define size_t int
extern char *strcpy(char *dest, const char *src);
extern char *strncpy(char *dest, const char *src, size_t n);
extern void *memset(void *s, int c, size_t n);
extern char *strcat(char *dest, const char *src);

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
#define INITIAL_MONSTER(x) (TOTAL_BADGES / sizeof(monsters))
#define ARRAYSIZE(x) (sizeof((x)) / sizeof((x)[0]))
#define SCREEN_XDIM 132
#define SCREEN_YDIM 132

static int screen_changed = 0;
static int smiley_x, smiley_y;
// static int initial_monster = INITIAL_MONSTER(G_sysData.badgeId);
static int current_monster = 0;
static int nmonsters = 0;

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
} monsters[] = {
	{"othermon", ARRAYSIZE(smiley_points), 0, 0, othermon_points},
	{"othermon", ARRAYSIZE(smiley_points), 0, 1, othermon_points},
	{"smileymon", ARRAYSIZE(smiley_points), 0, 1, smiley_points},
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
	char str[15], str2[15], title[15];
	int color;

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

	title[0] = '\0';
	color = WHITE;

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
    }
}

static void render_monster(void)
{
	int npoints, color;
	struct point *drawing;
	char *name;
	char id_string[3];
	itoa(id_string, current_monster, 10);

	change_menu_level(MONSTER_MENU);

	name = monsters[current_monster].name;
	drawing = monsters[current_monster].drawing;
	npoints = monsters[current_monster].npoints;
	color = monsters[current_monster].color;

	FbClear();
	FbWriteLine(name);
	FbWriteLine("\n");
	draw_object(drawing, npoints, color, smiley_x, smiley_y);
	FbSwapBuffers();
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
	printf("current item: %d\nmenu level: %d\ncurrent monster: %d\n menu item: %s\nn-menu-items: %d\n", 
	menu.current_item, menu_level, current_monster, menu.item[menu.current_item].text, menu.nitems);
}
#endif

static void check_the_buttons(void)
{

	int something_changed = 0;

	if (UP_BTN_AND_CONSUME)
	{
		something_changed = 1;
        menu_change_current_selection(-1);

		#ifdef __linux__
            print_menu_info();
		#endif
	}
	else if (DOWN_BTN_AND_CONSUME)
	{
		something_changed = 1;
        menu_change_current_selection(1);
		#ifdef __linux__
            print_menu_info();
		#endif
	}
	else if (BUTTON_PRESSED_AND_CONSUME)
	{
		if (menu_level == MONSTER_MENU)
		{
			current_monster = menu.current_item;
            render_monster();
		}

		if (menu_level == MAIN_MENU)
		{
            switch(menu.current_item){
				case 0:
                    change_menu_level(MONSTER_MENU);
                    something_changed = 1;
                    app_state = CHECK_THE_BUTTONS;
                    break;
                case 1:
                    #ifdef __linux__
                        printf("trade\n");
                    #endif
                    break;
                case 2:
                    app_state = EXIT_APP;
                    break;
			}
		}
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
		menu_add_item(monsters[i].name, RENDER_MONSTER, i);
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
	menu_add_item("EXIT GAME", EXIT_APP, 2);
	screen_changed = 1;
}

static void game_menu(void)
{
	draw_menu();
	if(menu_level == MAIN_MENU)
		app_state = RENDER_SCREEN;
	if(menu_level == MONSTER_MENU)
		setup_monster_menu();
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