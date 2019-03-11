
#ifdef __linux__

#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include "../linux/linuxcompat.h"
#include "lasertag-protocol.h"

#else

#include "colors.h"
#include "menu.h"
#include "buttons.h"
#include "ir.h"
#include "lasertag-protocol.h"
#include "flash.h"
#include "timer1_int.h" /* for wclock */

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
#define SENDDUMP 6 
#define FREEFORALL 7
#define TEAMBATTLE 8
#define ZOMBIES 9
#define CAPTUREBADGE 10
#define SENDNEWGAME 11 
#define INITGAMEDATA 12 

static void app_init(void);
static void render_screen(void);
static void check_the_buttons(void);
static void draw_menu(void);
static void exit_app(void);
static void send_hit(void);
static void send_dump(void);
static void free_for_all_game(void);
static void team_battle_game(void);
static void zombies_game(void);
static void capture_the_badge_game(void);
static void send_new_game(void);
static void init_game_data(void);

typedef void (*state_to_function_map_fn_type)(void);

static state_to_function_map_fn_type state_to_function_map[] = {
	app_init,
	draw_menu,
	render_screen,
	check_the_buttons,
	exit_app,
	send_hit,
	send_dump,
	free_for_all_game,
	team_battle_game,
	zombies_game,
	capture_the_badge_game,
	send_new_game,
	init_game_data,
};

static struct game_data {
#ifdef __linux__
	unsigned long absolute_start_time;
#else
	int absolute_start_time;
#endif
	unsigned char game_variant;
	unsigned int duration;
	unsigned int team;
	unsigned int game_id;
} game_data = {
	0,
	4,
	120,
	1,
	0,
};

static int app_state = INIT_APP_STATE;
static int something_changed = 0;
static int menu_choice = 0;

#define SCREEN_XDIM 132
#define SCREEN_YDIM 132

#define ARRAYSIZE(x) (sizeof(x) / sizeof((x)[0]))
struct menu_item {
	char *name;
	int next_state;
};

static struct menu_item m[] = {
	{ "HIT", SENDHIT },
	{ "REQUEST DUMP", SENDDUMP },
        { "FREE FOR ALL", FREEFORALL },
        { "TEAM BATTLE", TEAMBATTLE },
        { "ZOMBIES!", ZOMBIES },
        { "CAPTURE BADGE", CAPTUREBADGE },
	{ "SEND NEW GAME", SENDNEWGAME },
	{ "INIT GAME DATA", INITGAMEDATA  },
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

#define BASE_STATION_BADGE_ID G_sysData.badgeId

static void send_hit(void)
{
	unsigned int team_id;

	team_id = 5;

	send_a_packet(build_packet(1, 1, BADGE_IR_GAME_ADDRESS, BADGE_IR_BROADCAST_ID,
		(OPCODE_HIT << 12) | (G_sysData.badgeId << 4) | team_id));
	app_state = CHECK_THE_BUTTONS;
}

static void send_start_time(void)
{
	unsigned int start_time;
#ifdef __linux__
	struct timeval tv;

	gettimeofday(&tv, NULL);
	start_time = game_data.absolute_start_time - tv.tv_sec;
#else
	start_time = game_data.absolute_start_time - wclock.hour * 3600 - wclock.min * 60 - wclock.sec; 
#endif
	send_a_packet(build_packet(1, 1, BADGE_IR_GAME_ADDRESS, BADGE_IR_BROADCAST_ID,
			(OPCODE_SET_GAME_START_TIME << 12) | start_time));
}

static void send_duration(void)
{
	send_a_packet(build_packet(1, 1, BADGE_IR_GAME_ADDRESS, BADGE_IR_BROADCAST_ID,
		(OPCODE_SET_GAME_DURATION << 12) | game_data.duration));
}

static void send_variant(void)
{
	send_a_packet(build_packet(1, 1, BADGE_IR_GAME_ADDRESS, BADGE_IR_BROADCAST_ID,
		(OPCODE_SET_GAME_VARIANT << 12) | game_data.game_variant));
}

static void send_team(void)
{
	send_a_packet(build_packet(1, 1, BADGE_IR_GAME_ADDRESS, BADGE_IR_BROADCAST_ID,
		(OPCODE_SET_BADGE_TEAM << 12) | game_data.team));
}

static void send_id(void)
{
	send_a_packet(build_packet(1, 1, BADGE_IR_GAME_ADDRESS, BADGE_IR_BROADCAST_ID,
		(OPCODE_GAME_ID << 12) | game_data.game_id));
}

static void send_dump(void)
{
	send_a_packet(build_packet(1, 1, BADGE_IR_GAME_ADDRESS, BADGE_IR_BROADCAST_ID,
		(OPCODE_REQUEST_BADGE_DUMP << 12)));
	app_state = CHECK_THE_BUTTONS;
}

static void send_new_game(void)
{
	/* Send new game info to the badge */
	static int step_number = 0;

	switch (step_number) {
	case 0:
		send_duration();
		break;
	case 1:
		send_variant();
		break;
	case 2:
		send_team();
		break;
	case 3:
		send_id();
		break;
	case 4:
		send_start_time();
		break;
	default:
		break;
	}
	step_number++;
	if (step_number > 4) {
		step_number = 0;
		app_state = CHECK_THE_BUTTONS;
	}
}

static void free_for_all_game(void)
{
	game_data.game_variant = 0;
	app_state = CHECK_THE_BUTTONS;
}

static void team_battle_game(void)
{
	game_data.game_variant = 1;
	app_state = CHECK_THE_BUTTONS;
}

static void zombies_game(void)
{
	game_data.game_variant = 2;
	app_state = CHECK_THE_BUTTONS;
}

static void capture_the_badge_game(void)
{
	game_data.game_variant = 3;
	app_state = CHECK_THE_BUTTONS;
}

static void init_game_data(void)
{
#ifdef __linux__
	struct timeval tv;

	gettimeofday(&tv, NULL);
	game_data.absolute_start_time = tv.tv_sec + 120;
#else
	game_data.absolute_start_time = wclock.hour * 3600 + wclock.min * 60 + wclock.sec + 120;
#endif
	game_data.team = (game_data.team + 1) % 4;
	game_data.duration = 120;
	game_data.game_id = (game_data.game_id + 1) % 1024;
	app_state = CHECK_THE_BUTTONS;
}

static void draw_menu(void)
{
	int i;
	char item[20];

	FbClear();
	for (i = 0; i < MAX_MENU_CHOICES; i++) {
		if (i == menu_choice) {
			FbMove(5, i * 10);
			FbWriteLine("=>");
		}
		strcpy(item, m[i].name);
		FbMove(20, i * 10);
		FbWriteLine(item);
	}
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
			menu_choice = MAX_MENU_CHOICES - 1;
		something_changed = 1;
	} else if (DOWN_BTN_AND_CONSUME) {
		menu_choice++;
		if (menu_choice >= MAX_MENU_CHOICES)
			menu_choice = 0;
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

static void ir_packet_callback(struct IRpacket_t packet)
{
	static int recordnum = 0;
	static int recordcount = 0;
	/* Interrupts will be already disabled when this is called. */
#ifdef __linux__
	unsigned int v = packet.v;
	int opcode = ((v >> 12) & 0x0f);
	switch (opcode) {
	case OPCODE_SET_GAME_START_TIME:
	case OPCODE_SET_GAME_DURATION:
	case OPCODE_HIT:
	case OPCODE_REQUEST_BADGE_DUMP:
 		/* 1. Base station requests info from badge: OPCODE_REQUEST_BADGE_DUMP
 		 * 2. Badge respondes with OPCODE_BADGE_IDENTITY
 		 * 3. Badge responds with OPCODE_GAME_ID
 		 * 4. Badge responds with OPCODE_BADGE_RECORD_COUNT
 		 * 5. Badge responds with triplets of OPCODE_BADGE_UPLOAD_HIT_RECORD_BADGE_ID,
 		 *    OPCODE_BADGE_UPLOAD_HIT_RECORD_TIMESTAMP, and OPCODE_SET_BADGE_TEAM.
 		 */
	case OPCODE_SET_GAME_VARIANT:
		fprintf(stderr, "Received recognized but inappropriate opcode: %d\n", opcode);
		break;
	case OPCODE_BADGE_IDENTITY:
		fprintf(stderr, "dump: BADGE IDENTITY: 0x%08x\n", opcode & 0x01ff);
		break;
	case OPCODE_GAME_ID:
		fprintf(stderr, "dump: GAME ID: 0x%08x\n", (opcode & 0x0fff));
		break;
	case OPCODE_BADGE_RECORD_COUNT:
		fprintf(stderr, "dump: RECORD COUNT: 0x%08x\n", (opcode & 0x0fff));
		recordcount = opcode & 0x0fff;
		break;
	case OPCODE_BADGE_UPLOAD_HIT_RECORD_BADGE_ID:
		fprintf(stderr, "%c %d: BADGE_ID 0x%08x", recordnum >= recordcount ? '+' : '-',
				recordnum, (opcode & 0x01ff));
		break;
	case OPCODE_BADGE_UPLOAD_HIT_RECORD_TIMESTAMP:
		fprintf(stderr, "TIME 0x%08x", (opcode & 0x0ffff));
		break;
	case OPCODE_SET_BADGE_TEAM:
		fprintf(stderr, "TEAM 0x%08x\n", (opcode & 0x0f));
		recordnum++;
		break;
	default:
		fprintf(stderr, "Unrecognized opcode: 0x%08x, packet = 0x%08x\n", opcode, v);
		break;
	}
#endif
}

#ifndef __linux__
static void (*old_callback)(struct IRpacket_t) = 0;
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

static void exit_app(void)
{
	unregister_ir_packet_callback();
	app_state = INIT_APP_STATE;
	returnToMenus();
}

static void app_init(void)
{
	register_ir_packet_callback(ir_packet_callback);
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

	char *serial_port = NULL;
#define IRXMIT_UDP_PORT 12345
#define LASERTAG_UDP_PORT 12346

	if (argc >= 2)
		serial_port = argv[1];
	setup_linux_ir_simulator(serial_port, LASERTAG_UDP_PORT, IRXMIT_UDP_PORT);
	start_gtk(&argc, &argv, irxmit_cb, 30);
	return 0;
}

#endif
