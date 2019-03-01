
#ifdef __linux__

#include <stdio.h>
#include <string.h>

#include "../linux/linuxcompat.h"
#include "lasertag-protocol.h"

#else

#include "colors.h"
#include "menu.h"
#include "buttons.h"
#include "ir.h"
#include "lasertag-protocol.h"

/* TODO: I shouldn't have to declare these myself. */
#define size_t int
extern char *strcpy(char *dest, const char *src);
extern char *strncpy(char *dest, const char *src, size_t n);
extern void *memset(void *s, int c, size_t n);
extern char *strcat(char *dest, const char *src);


#endif

#define INIT_APP_STATE 0
#define DRAW_MENU 1
#define RENDER_SCREEN 2
#define CHECK_THE_BUTTONS 3
#define EXIT_APP 4
#define SENDHIT 5
#define SENDSTARTTIME 6
#define SENDDURATION 7
#define SENDVARIANT 8
#define SENDTEAM 9
#define SENDID 10
#define SENDDUMP 11

static void app_init(void);
static void render_screen(void);
static void check_the_buttons(void);
static void draw_menu(void);
static void exit_app(void);
static void send_hit(void);
static void send_start_time(void);
static void send_duration(void);
static void send_variant(void);
static void send_team(void);
static void send_id(void);
static void send_dump(void);

typedef void (*state_to_function_map_fn_type)(void);

static state_to_function_map_fn_type state_to_function_map[] = {
	app_init,
	draw_menu,
	render_screen,
	check_the_buttons,
	exit_app,
	send_hit,
	send_start_time,
	send_duration,
	send_variant,
	send_team,
	send_id,
	send_dump,
};

static int app_state = INIT_APP_STATE;
static int something_changed = 0;
static int menu_choice = 0;
static volatile unsigned int last_packet_out = 0;

#define SCREEN_XDIM 132
#define SCREEN_YDIM 132

#define ARRAYSIZE(x) (sizeof(x) / sizeof((x)[0]))
struct menu_item {
	char *name;
	int next_state;
};

static struct menu_item m[] = {
	{ "HIT", SENDHIT },
	{ "GAME STRT TIME", SENDSTARTTIME },
	{ "GAME DURATION", SENDDURATION },
	{ "GAME VARIANT", SENDVARIANT },
	{ "TEAM", SENDTEAM },
	{ "ID", SENDID },
	{ "REQUEST DUMP", SENDDUMP },
	{ "EXIT\n", EXIT_APP },
};

#define MAX_MENU_CHOICES ARRAYSIZE(m)

static unsigned int build_packet(unsigned char cmd, unsigned char start,
			unsigned char address, unsigned short badge_id, unsigned short payload)
{
	return ((cmd & 0x01) << 31) |
		((start & 0x01) << 30) |
		((address & 0x01f) << 25) |
		((badge_id & 0x1ff) << 16) |
		(payload);
}

static void send_a_packet(unsigned int packet)
{
	union IRpacket_u p;

	p.v = packet;
	last_packet_out = packet;
	something_changed = 1;
	IRqueueSend(p);

#ifdef __linux__
	printf("\nSent packet: %08x\n", packet);
	printf("      cmd: 0x%01x\n", (packet >> 31) & 0x01);
	printf("    start: 0x%01x\n", (packet >> 30) & 0x01);
	printf("  address: 0x%02x\n", (packet >> 25) & 0x1f);
	printf(" badge ID: 0x%03x\n", (packet >> 16) & 0x1ff);
	printf("  payload: 0x%04x\n\n", packet & 0x0ffff);
#endif

}

static void send_hit(void)
{
	unsigned int badge_id, team_id;

	badge_id = 15;
	team_id = 5;

	send_a_packet(build_packet(1, 1, BADGE_IR_GAME_ADDRESS, badge_id,
		(OPCODE_HIT << 12) | team_id));
	app_state = CHECK_THE_BUTTONS;
}

#define BASE_STATION_BADGE_ID 99

static void send_start_time(void)
{
	unsigned int start_time;
	unsigned short badge_id = BASE_STATION_BADGE_ID;

	start_time = 30;
	send_a_packet(build_packet(1, 1, BADGE_IR_GAME_ADDRESS, badge_id,
			(OPCODE_SET_GAME_START_TIME << 12) | start_time));
	app_state = CHECK_THE_BUTTONS;
}

static void send_duration(void)
{
	unsigned int duration;
	unsigned short badge_id = BASE_STATION_BADGE_ID;

	duration = 30;
	send_a_packet(build_packet(1, 1, BADGE_IR_GAME_ADDRESS, badge_id,
		(OPCODE_SET_GAME_DURATION << 12) | duration));
	app_state = CHECK_THE_BUTTONS;
}

static void send_variant(void)
{
	unsigned int game_variant;
	unsigned short badge_id = BASE_STATION_BADGE_ID;

	game_variant = 4;
	send_a_packet(build_packet(1, 1, BADGE_IR_GAME_ADDRESS, badge_id,
		(OPCODE_SET_GAME_VARIANT << 12) | game_variant));
	app_state = CHECK_THE_BUTTONS;
}

static void send_team(void)
{
	unsigned int team_id;
	unsigned short badge_id = BASE_STATION_BADGE_ID;

	team_id = 1;
	send_a_packet(build_packet(1, 1, BADGE_IR_GAME_ADDRESS, badge_id,
		(OPCODE_SET_BADGE_TEAM << 12) | team_id));
	app_state = CHECK_THE_BUTTONS;
}

static void send_id(void)
{
	unsigned int game_id;
	unsigned short badge_id = BASE_STATION_BADGE_ID;

	game_id = 99;
	send_a_packet(build_packet(1, 1, BADGE_IR_GAME_ADDRESS, badge_id,
		(OPCODE_GAME_ID << 12) | game_id));
	app_state = CHECK_THE_BUTTONS;
}

static void send_dump(void)
{
	send_a_packet(build_packet(1, 1, BADGE_IR_GAME_ADDRESS, BASE_STATION_BADGE_ID,
		(OPCODE_REQUEST_BADGE_DUMP << 12)));
	app_state = CHECK_THE_BUTTONS;
}

static void to_hex(char *buffer, unsigned int v)
{
	int i;
	int nybble;
	char *a = "0123456789ABCDEF";

	for (i = 0; i < 8; i++) {
		nybble = v & 0x0f;
		buffer[7 - i] = a[nybble];
		v = v >> 4;
	}
	buffer[8] = '\0';
}

static void draw_menu(void)
{
	int i;
	char item[20];

	FbClear();
	FbColor(WHITE);
	for (i = 0; i < MAX_MENU_CHOICES; i++) {
		if (i == menu_choice) {
			FbMove(5, i * 10);
			FbWriteLine("=>");
		}
		strcpy(item, m[i].name);
		FbMove(20, i * 10);
		FbWriteLine(item);
	}
	FbMove(5, 120);
	to_hex(item, last_packet_out);
	FbWriteLine(item);
	app_state = RENDER_SCREEN;
}

static void render_screen(void)
{
	if (something_changed)
		FbSwapBuffers();
	something_changed = 0;
	app_state = CHECK_THE_BUTTONS;
}

static void check_the_buttons(void)
{
	if (UP_BTN_AND_CONSUME) {
		menu_choice--;
		if (menu_choice < 0)
			menu_choice = 0;
		something_changed = 1;
	} else if (DOWN_BTN_AND_CONSUME) {
		menu_choice++;
		if (menu_choice >= MAX_MENU_CHOICES)
			menu_choice = MAX_MENU_CHOICES - 1;
		something_changed = 1;
	} else if (LEFT_BTN_AND_CONSUME) {
	} else if (RIGHT_BTN_AND_CONSUME) {
	} else if (BUTTON_PRESSED_AND_CONSUME) {
		app_state = m[menu_choice].next_state;
	}
	if (something_changed && app_state == CHECK_THE_BUTTONS)
		app_state = DRAW_MENU;
        return;
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
	something_changed = 1;
	app_state = DRAW_MENU;
}

int irxmit_cb(void)
{
	state_to_function_map[app_state]();
	return 0;
}

#ifdef __linux__

int main(int argc, char *argv[])
{
	start_gtk(&argc, &argv, irxmit_cb, 30);
	return 0;
}

#endif
