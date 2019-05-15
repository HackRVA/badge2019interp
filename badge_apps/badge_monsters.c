
#ifdef __linux__
#include <string.h>

#include "../linux/linuxcompat.h"

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
#define CHECK_THE_BUTTONS 3
#define EXIT_APP 4

static void app_init(void);
static void game_menu(void);
static void render_screen(void);
static void check_the_buttons(void);
static void exit_app(void);

typedef void (*state_to_function_map_fn_type)(void);

static state_to_function_map_fn_type state_to_function_map[] = {
	app_init,
	game_menu,
	render_screen,
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

static int app_state = INIT_APP_STATE;

static struct point {
    signed char x, y;
};

static struct point smiley_points[] = 
#include "smileymon.h"

static struct point othermon_points[] = 
#include "othermon.h"

static struct monster {
	char name[20];
	int npoints;
	short status;
	int color;
	struct point *drawing;
} monsters[] = {
	{ "othermon", ARRAYSIZE(smiley_points), 0, 0, othermon_points },
	{ "othermon", ARRAYSIZE(smiley_points), 0, 1, othermon_points },
	{ "smileymon", ARRAYSIZE(smiley_points), 0, 1, smiley_points },
};

static void draw_object(struct point drawing[], int npoints, int color, int x, int y)
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

struct menu_item {
	char text[15];
	state_to_function_map_fn_type next_state;
	unsigned char cookie;
};

static struct menu {
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
	menu.menu_active = 0;
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
	for (i = first_item; i <= last_item; i++) {
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

static void setup_main_menu(void)
{
	// int initial_monster = INITIAL_MONSTER(G_sysData.badgeId);
	menu_clear();
	menu.menu_active = 1;
	strcpy(menu.title, "Badge Monsters");
	menu_add_item("RENDER_SCREEN", RENDER_SCREEN, 0);
	menu_add_item("EXIT GAME", EXIT_APP, 1);
	screen_changed = 1;
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

static void render_monster(int monsterID){
	int npoints, color;
	struct point *drawing;
	char* name;
	char id_string[3];
	itoa(id_string, monsterID, 10);


	name = monsters[monsterID].name;
	drawing = monsters[monsterID].drawing;
	npoints = monsters[monsterID].npoints;
	color = monsters[monsterID].color;
	
	FbClear();
	// FbWriteLine(id_string);
	FbWriteLine(name);
	FbWriteLine("\n");
	draw_object(drawing, npoints, color, smiley_x, smiley_y);
	FbSwapBuffers();
}

static void render_screen(void)
{
	app_state = CHECK_THE_BUTTONS;
	if (!screen_changed)
		return;
	FbPushBuffer();
	screen_changed = 0;
}

static void check_the_buttons(void)
{
	const int left_limit = SCREEN_XDIM / 3;
	const int right_limit = 2 * SCREEN_XDIM / 3;
	const int top_limit = SCREEN_YDIM / 3;
	const int bottom_limit = 2 * SCREEN_YDIM / 3;

	int something_changed = 0;

	if (UP_BTN_AND_CONSUME) {
		something_changed = 1;
		if (menu.menu_active)
			menu_change_current_selection(-1);
	} else if (DOWN_BTN_AND_CONSUME) {
		something_changed = 1;
		if (menu.menu_active)
			menu_change_current_selection(1);
	} else if (LEFT_BTN_AND_CONSUME) {
		smiley_x -= 1;
		something_changed = 1;
	} else if (RIGHT_BTN_AND_CONSUME) {
		smiley_x += 1;
		something_changed = 1;
	} else if (BUTTON_PRESSED_AND_CONSUME) {
		if (menu.menu_active) {
			app_state = CHECK_THE_BUTTONS;
			render_monster(0);
			}
		something_changed = 1;
		menu_change_current_selection(-1);
	}
	if (smiley_x < left_limit)
		smiley_x = left_limit;
	if (smiley_x > right_limit)
		smiley_x = right_limit;
	if (smiley_y < top_limit)
		smiley_y = top_limit;
	if (smiley_y > bottom_limit)
		smiley_y = bottom_limit;
	if (something_changed && app_state == CHECK_THE_BUTTONS)
		app_state = GAME_MENU;
        return;
}

static void game_menu(void){
	menu.menu_active = 1;
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
	setup_main_menu();
	app_state = GAME_MENU;
	screen_changed = 1;
	smiley_x = SCREEN_XDIM / 2;
	smiley_y = SCREEN_XDIM / 2;
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
