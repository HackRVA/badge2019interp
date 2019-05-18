/*

This is code for a kind of "laser-tag"-ish game meant to be played
with little PIC32 badges that have IR transmitters and receivers.
If the code seems a little strange, it's due to the environment this
code must run in.

*/

#ifdef __linux__
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

#include "../linux/linuxcompat.h"

#define DISABLE_INTERRUPTS do { disable_interrupts(); } while (0)
#define ENABLE_INTERRUPTS do { enable_interrupts(); } while (0)

#else

#include "colors.h"
#include "menu.h"
#include "buttons.h"
#include "ir.h"
#include "flash.h" /* for G_sysData */
#include "timer1_int.h"

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

/* TODO: Is there a canonical header I should include to get the screen dimensions? */
#define SCREEN_XDIM 132
#define SCREEN_YDIM 132

#define DISABLE_INTERRUPTS
#define ENABLE_INTERRUPTS

#endif

#include "lasertag-protocol.h"
#include "build_bug_on.h"

#define ARRAYSIZE(x) (sizeof((x)) / sizeof((x)[0]))

/* These need to be protected from interrupts. */
#define QUEUE_SIZE 5
static int queue_in;
static int queue_out;
static int packet_queue[QUEUE_SIZE] = { 0 };

static int screen_changed = 0;
#define NO_GAME_START_TIME -1000000
static volatile int seconds_until_game_starts = NO_GAME_START_TIME;
static volatile unsigned int game_id = -1;
static volatile int game_duration = -1;
static volatile int game_start_timestamp = -1;
static volatile int current_time = -1;
static volatile int team = -1;
static volatile int game_variant = GAME_VARIANT_NONE;
static volatile int suppress_further_hits_until = -1;
static const char *game_type[] = {
	"FREE FOR ALL",
	"TEAM BATTLE",
	"ZOMBIES!",
	"CAPTURE BADGE",
};

#define MAX_HIT_TABLE_ENTRIES 256
static struct hit_table_entry {
	unsigned short badgeid;
	unsigned short timestamp;
	unsigned char team;
} hit_table[MAX_HIT_TABLE_ENTRIES];
static int nhits = 0;

static struct powerup {
	char *name;
	char obtained;
} powerup[NUM_LASERTAG_POWERUPS] = {
#define RESILIENCE 0
	{ "RESILIENCE", 0 }, /* Deadtime will be 5 seconds instead of 30 */
#define IMMUNITY 1
	{ "IMMUNITY", 0 }, /* Immunity from one hit. */
};
static int continuous_powerup_granting_enabled = 0;

static struct rgbcolor {
	unsigned char r, g, b;
} team_color[] = {
	{ 128, 0, 0 }, /* red */
	{ 0, 128, 0 }, /* green */
	{ 0, 0, 128 }, /* blue */
	{ 128, 128, 0 }, /*  yellow */
	{ 255, 128, 0 }, /*  orange */
	{ 128, 0, 128 }, /* purple */
	{ 0, 128, 128 }, /* blue green */
	{ 128, 128, 128 }, /* white */
	{ 128, 128, 128 } /* white */
};

extern char username[10];

/* Builds up a 32 bit badge packet.
 * 1 bit for start
 * 1 bit for cmd,
 * 5 bits for addr
 * 9 bits for badge_id of recipient, or 0 for broadcast,
 * 16 bits for payload
 */
static unsigned int build_ir_packet(unsigned char start, unsigned char cmd,
		unsigned char addr, unsigned short badge_id, unsigned short payload)
{
	unsigned int packet;

	BUILD_ASSERT(sizeof(unsigned int) == 4); /* these generate no code */
	BUILD_ASSERT(sizeof(unsigned short) == 2);

	packet = ((start & 0x01) << 31) | ((cmd & 0x01) << 30) | ((addr & 0x01f) << 25) |
		 ((badge_id & 0x1ff) << 16) | (payload & 0x0ffff);
	return packet;
}

static unsigned char __attribute__((unused)) get_start_bit(unsigned int packet)
{
	return (unsigned char) ((packet >> 31) & 0x01);
}

static unsigned char __attribute__((unused)) get_cmd_bit(unsigned int packet)
{
	return (unsigned char) ((packet >> 30) & 0x01);
}

static unsigned char __attribute__((unused)) get_addr_bits(unsigned int packet)
{
	return (unsigned char) ((packet >> 25) & 0x01f);
}

static unsigned char get_shooter_badge_id_bits(unsigned int packet)
{
	return (unsigned char) ((packet >> 3) & 0x1ff);
}

static unsigned short get_payload(unsigned int packet)
{
	return (unsigned short) (packet & 0x0ffff);
}

typedef void (*game_state_function)(void);

static void initial_state(void);
static void game_exit(void);
static void game_main_menu(void);
static void game_process_button_presses(void);
static void game_screen_render(void);
static void game_shoot(void);
static void game_confirm_exit(void);
static void game_exit_abandoned(void);
static void game_dump_data(void);
static void game_grant_powerup(void);
static void game_grant_continuous_powerup(void);
static void game_stop_powerups(void);

static enum game_state_type {
	INITIAL_STATE = 0,
	GAME_EXIT = 1,
	GAME_MAIN_MENU = 2,
	GAME_PROCESS_BUTTON_PRESSES = 3,
	GAME_SCREEN_RENDER = 4,
	GAME_SHOOT = 5,
	GAME_CONFIRM_EXIT = 6,
	GAME_EXIT_ABANDONED = 7,
	GAME_DUMP_DATA = 8,
	GAME_GRANT_POWERUP = 9,
	GAME_GRANT_CONTINUOUS_POWERUPS = 10,
	GAME_STOP_POWERUPS = 11
} game_state = INITIAL_STATE;

/* Note, game_state_fn[] array must have an entry for every value
   defined by enum game_state_type.  */
static game_state_function game_state_fn[] = {
	initial_state,
	game_exit,
	game_main_menu,
	game_process_button_presses,
	game_screen_render,
	game_shoot,
	game_confirm_exit,
	game_exit_abandoned,
	game_dump_data,
	game_grant_powerup,
	game_grant_continuous_powerup,
	game_stop_powerups,
};

struct menu_item {
	char text[15];
	enum game_state_type next_state;
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

static void menu_add_item(char *text, enum game_state_type next_state, unsigned char cookie)
{
	int i;

	if (menu.nitems >= ARRAYSIZE(menu.item))
		return;

	i = menu.nitems;
	strncpy(menu.item[i].text, text, sizeof(menu.item[i].text) - 1);
	menu.item[i].next_state = next_state;
	menu.item[i].cookie = cookie;
	menu.nitems++;
}

static void to_hex(unsigned int n, char buf[])
{
	int i, nybble;
	int shift;
	const char *dig = "0123456789ABCDEFG";

	for (i = 0; i < 8; i++) {
		shift = (8 - i) * 4 - 4;
		nybble = (n >> shift) & 0x0f;
		buf[i] = dig[nybble];
	}
	buf[8] = '\0';
}

static int all_game_data_received(void);

static void draw_menu(void)
{
	int i, y, first_item, last_item;
	char str[15], str2[15], title[15], timecode[15], badgeidstr[20];
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
		if (y > 100)
			break;
	}

	FbColor(GREEN);
	FbHorizontalLine(5, SCREEN_YDIM / 2 - 2, SCREEN_XDIM - 5, SCREEN_YDIM / 2 - 2);
	FbHorizontalLine(5, SCREEN_YDIM / 2 + 10, SCREEN_XDIM - 5, SCREEN_YDIM / 2 + 10);
	FbVerticalLine(5, SCREEN_YDIM / 2 - 2, 5, SCREEN_YDIM / 2 + 10);
	FbVerticalLine(SCREEN_XDIM - 5, SCREEN_YDIM / 2 - 2, SCREEN_XDIM - 5, SCREEN_YDIM / 2 + 10);


	if (all_game_data_received()) {
		title[0] = '\0';
		timecode[0] = '\0';
		color = WHITE;
		if (seconds_until_game_starts == NO_GAME_START_TIME) {
			suppress_further_hits_until = -1;
			strcpy(title, "GAME OVER");
			strcpy(timecode, "");
			color = YELLOW;
		} else {
			if (seconds_until_game_starts > 0) {
				strcpy(title, "GAME STARTS IN");
				itoa(timecode, seconds_until_game_starts, 10);
				strcat(timecode, " SECS");
				color = YELLOW;
			} else {
				if (seconds_until_game_starts > -game_duration) {
					strcpy(title, "TIME LEFT:");
					itoa(timecode, game_duration + seconds_until_game_starts, 10);
					strcat(timecode, " SECS");
					color = WHITE;
				} else {
					suppress_further_hits_until = -1;
					strcpy(title, "GAME OVER");
					strcpy(timecode, "");
					game_duration = -1;
					seconds_until_game_starts = NO_GAME_START_TIME;
					screen_changed = 1;
					color = YELLOW;
				}
			}
		}
		if (suppress_further_hits_until > 0) { /* have been hit recently */
			static int old_deadtime = 0;
			color = RED;
			FbColor(color);
			FbMove(10, 40);
			strcpy(str, "DEAD TIME:");
			itoa(str2, suppress_further_hits_until - current_time, 10);
			if (old_deadtime != suppress_further_hits_until - current_time) {
				old_deadtime = suppress_further_hits_until - current_time;
				screen_changed = 1;
			}
			strcat(str, str2);
			FbWriteLine(str);
		}
		FbColor(color);
		if (game_variant != GAME_VARIANT_NONE) {
			FbMove(10, 10);
			strcpy(str2, game_type[game_variant % ARRAYSIZE(game_type)]);
			FbWriteLine(str2);
		}
		if (team >= 0) {
			FbMove(10, 20);
			itoa(str, team, 10);
			strcpy(str2, "TEAM:");
			strcat(str2, str);
			FbWriteLine(str2);
		}
		FbMove(10, 30);
		strcpy(str, "HITS:");
		itoa(str2, nhits, 10);
		strcat(str, str2);
		FbWriteLine(str);
		FbMove(10, 100);
		FbWriteLine(title);
		FbMove(10, 110);
		FbWriteLine(timecode);

#define LASERTAG_DISPLAY_CURRENT_TIME 0
#if LASERTAG_DISPLAY_CURRENT_TIME
		/* Draw the current time */
		itoa(str2, wclock.hour, 10);
		strcpy(timecode, str2);
		strcat(timecode, ":");
		itoa(str2, wclock.min, 10);
		strcat(timecode, str2);
		strcat(timecode, ":");
		itoa(str2, wclock.sec, 10);
		strcat(timecode, str2);
		FbMove(10, 120);
		FbWriteLine(timecode);
#endif
	} else {
		/* else not all game data received */
		FbMove(10, 20);
		FbWriteLine("AWAITING DATA");
		FbMove(10, 30);
		FbWriteLine("FROM BASE");
		FbMove(10, 40);
		FbWriteLine("STATION");
	}
	to_hex(G_sysData.badgeId, badgeidstr);
	FbMove(131 - 4 * 8, 131 - 10);
	FbWriteLine(badgeidstr + 4); /* only print last 4 digits, it's a 16 bit number. */

	/* Draw any powerups we might have accumulated... */
	for (i = 0; i < ARRAYSIZE(powerup); i++)
		if (powerup[i].obtained) {
			char buf[2];
			buf[0] = powerup[i].name[0];
			buf[1] = '\0';
			FbMove(10 + 10 * i, 131 - 10);
			FbWriteLine(buf);
		}

	game_state = GAME_SCREEN_RENDER;
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

static int is_vendor_badge(unsigned short badgeId)
{
	return (G_sysData.badgeId & 0x1ff) < NUM_VENDOR_BADGES && (G_sysData.badgeId & 0x1ff) >= 0;
}

static void setup_main_menu(void)
{
	menu_clear();
	menu.menu_active = 1;
	strcpy(menu.title, "");
	menu_add_item("SHOOT", GAME_SHOOT, 0);
	if (is_vendor_badge(G_sysData.badgeId)) {
		menu_add_item("GIVE 1 POWERUP", GAME_GRANT_POWERUP, 0);
		if (continuous_powerup_granting_enabled)
			menu_add_item("STOP POWERUPS", GAME_STOP_POWERUPS, 0);
		else
			menu_add_item("AUTO POWERUPS", GAME_GRANT_CONTINUOUS_POWERUPS, 0);
	}
	menu_add_item("EXIT GAME", GAME_CONFIRM_EXIT, 0);
	screen_changed = 1;
}

static void setup_confirm_exit_menu(void)
{
	menu_clear();
	menu.menu_active = 1;
	strcpy(menu.title, "REALLY QUIT?");
	menu_add_item("DO NOT QUIT", GAME_EXIT_ABANDONED, 0);
	menu_add_item("REALLY QUIT", GAME_EXIT, 0);
	screen_changed = 1;
}

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

static void initial_state(void)
{
	FbInit();
	register_ir_packet_callback(ir_packet_callback);
	queue_in = 0;
	queue_out = 0;
	setup_main_menu();
	game_state = GAME_MAIN_MENU;
	screen_changed = 1;
}

static void game_exit(void)
{
	unregister_ir_packet_callback();
	returnToMenus();
	game_state = INITIAL_STATE;
}

static void game_main_menu(void)
{
	menu.menu_active = 1;
	draw_menu();
	game_state = GAME_SCREEN_RENDER;
}

static void game_confirm_exit(void)
{
	setup_confirm_exit_menu();
	game_state = GAME_MAIN_MENU;
}

static void game_exit_abandoned(void)
{
	setup_main_menu();
	game_state = GAME_MAIN_MENU;
}

static void button_pressed()
{
	if (menu.menu_active) {
		game_state = menu.item[menu.current_item].next_state;
		menu.chosen_cookie = menu.item[menu.current_item].cookie;
		menu.menu_active = 0;
		return;
	}
}

#ifndef __linux__
static int get_time(void)
{
#if LASERTAG_DISPLAY_CURRENT_TIME
	static int previous_sec = 0;
	/* I guess this will wrap-around at midnight. Do we actually care about that? */
	if (previous_sec != wclock.sec) {
		previous_sec = wclock.sec;
		screen_changed = 1;
	}
#endif
	return 3600 * (int) wclock.hour + 60 * (int) wclock.min + (int) wclock.sec;
}
#endif

static void set_game_start_timestamp(int time)
{
#ifdef __linux__
	struct timeval tv;

	gettimeofday(&tv, NULL);
	game_start_timestamp = tv.tv_sec + time;
#else
	game_start_timestamp = get_time() + time;
#endif
}

static void process_hit(unsigned int packet)
{
	int timestamp;
	unsigned char shooter_team = (get_payload(packet) & 0x07);
	unsigned short badgeid = get_shooter_badge_id_bits(packet);
	timestamp = current_time - game_start_timestamp;
	if (timestamp < 0) /* game has not started yet  */
		return;
	if (seconds_until_game_starts == NO_GAME_START_TIME)
		return;
	if (current_time < suppress_further_hits_until)
		return;

	switch (game_variant) {
	case GAME_VARIANT_ZOMBIE:
		if (team == shooter_team) /* exclude friendly fire */
			return;
		if (shooter_team == 1) { /* shooter is zombie? */
			team = 1; /* we become zombie */
			/* Note: we do not transfer this info to the base station */
			/* Instead the base station must re-construct this data from */
			/* the hit records (if it even needs to). */
		}
		break;
	case GAME_VARIANT_TEAM_BATTLE:
	case GAME_VARIANT_CAPTURE_THE_BADGE: /* TODO: implement capt the badge.  Too hard, not gonna. */
		if (team == shooter_team) /* exclude friendly fire */
			return;
	case GAME_VARIANT_FREE_FOR_ALL:
		break;
	case GAME_VARIANT_NONE:
		/* fall through */
	default:
		return; /* hits have no effect if a known game is not in play */
	}

	/* Dodge via immunity? */
	if (powerup[IMMUNITY].obtained) {
		screen_changed = 1;
		powerup[IMMUNITY].obtained = 0;
#ifdef __linux__
		fprintf(stderr, "lasertag: Used up hit immunity powerup\n");
#endif
		return;
	}

	hit_table[nhits].badgeid = badgeid;
	hit_table[nhits].timestamp = (unsigned short) timestamp;
	hit_table[nhits].team = shooter_team;
	nhits++;
	setNote(50, 4000); /* beep upon receipt of hit. */
	screen_changed = 1;
	if (nhits >= MAX_HIT_TABLE_ENTRIES)
		nhits = 0;

	if (powerup[RESILIENCE].obtained) {
		suppress_further_hits_until = current_time + 5;
		powerup[RESILIENCE].obtained = 0;
		screen_changed = 1;
#ifdef __linux__
		fprintf(stderr, "lasertag: Used up resilience powerup\n");
#endif
	} else {
		suppress_further_hits_until = current_time + 30;
	}
}

static void process_vendor_powerup(unsigned int packet)
{
	unsigned short badgeid = packet & 0x1ff;
	if (badgeid < 1 || badgeid > ARRAYSIZE(powerup) + 1)
		return;
#ifdef __linux__
	fprintf(stderr, "lasertag: Received powerup %d\n", badgeid - 1); 
#endif
	setNote(70, 4000);
	if (!powerup[badgeid - 1].obtained) {
		powerup[badgeid - 1].obtained = 1;
		screen_changed = 1;
	}
}

static void send_ir_packet(unsigned int packet)
{
	union IRpacket_u p;

	p.v = packet;
	p.v = packet | (1 << 30); /* Set command bit on outgoing packets */

	IRqueueSend(p);
}

static void send_badge_identity_packet(void)
{
	send_ir_packet(build_ir_packet(1, 1, BADGE_IR_GAME_ADDRESS, BADGE_IR_BROADCAST_ID,
		(OPCODE_BADGE_IDENTITY << 12) | (G_sysData.badgeId & 0x01ff)));
}

static void send_game_id_packet(unsigned int game_id)
{
	send_ir_packet(build_ir_packet(1, 1, BADGE_IR_GAME_ADDRESS, BADGE_IR_BROADCAST_ID,
		(OPCODE_GAME_ID << 12) | (game_id & 0x0fff)));
}

static void send_badge_record_count(unsigned int nhits)
{
	send_ir_packet(build_ir_packet(1, 1, BADGE_IR_GAME_ADDRESS, BADGE_IR_BROADCAST_ID,
		(OPCODE_BADGE_RECORD_COUNT << 12) | (nhits & 0x0fff)));
}

static void send_badge_upload_hit_record_badge_id(struct hit_table_entry *h)
{
	send_ir_packet(build_ir_packet(1, 1, BADGE_IR_GAME_ADDRESS, BADGE_IR_BROADCAST_ID,
		(OPCODE_BADGE_UPLOAD_HIT_RECORD_BADGE_ID << 12) | (h->badgeid & 0x01ff)));
}

static void send_badge_upload_hit_record_timestamp(struct hit_table_entry *h)
{
	send_ir_packet(build_ir_packet(1, 1, BADGE_IR_GAME_ADDRESS, BADGE_IR_BROADCAST_ID,
		(OPCODE_BADGE_UPLOAD_HIT_RECORD_TIMESTAMP << 12) | (h->timestamp & 0x0fff)));
}

static void send_badge_upload_hit_record_team(struct hit_table_entry *h)
{
	send_ir_packet(build_ir_packet(1, 1, BADGE_IR_GAME_ADDRESS, BADGE_IR_BROADCAST_ID,
		(OPCODE_SET_BADGE_TEAM << 12) | (h->team & 0x07)));
}

static void send_badge_username_packet(unsigned short encoded)
{
	send_ir_packet(build_ir_packet(1, 1, BADGE_IR_GAME_ADDRESS, BADGE_IR_BROADCAST_ID,
		(OPCODE_USERNAME_DATA << 12) | encoded));
}

static unsigned short encode_username_substring(char *substring, int length)
{
	int i, shift;
	unsigned short encoded;
	unsigned short answer = 0;

	if (length > 2)
		length = 2;
	shift = 5;
	for (i = 0; i < length; i++) {
		if (substring[i] >= 'A' && substring[i] <= 'Z') {
			encoded = substring[i] - 'A';
		} else if (substring[i] == '_') {
			encoded = 26;
		} else {
			encoded = 27;
		}
		answer = answer | (encoded << shift);
		shift = shift - 5;
	}
	if (length == 1)
		answer = answer | 27;
#ifdef __linux__
	printf("Encoded '%c%c' as 0x%hx\n", substring[0], substring[1], answer);
#endif
	return answer;
}

#ifdef __linux__
static void decode_username_fragment(unsigned short s)
{
	int c[2], i;

	c[0] = (s >> 5) & 0x1f;
	c[1] = s & 0x1f;
	for (i = 0; i < 2; i++) {
		if (c[i] >= 0 && c[i] <= 25)
		c[i] = c[i] + 'A';
		if (c[i] == 27)
			c[i] = '\0';
		if (c[i] == 26)
			c[i] = '_';
	}
	printf("0x%hx decodes to %c%c\n", s, c[0], c[1]);
}
#endif

static void game_dump_data(void)
{
	static int delay = 0;
#ifndef __linux__
	const int delay_count = 20000;
#else
	const int delay_count = 1;
#endif
	static int record_num = 0;

	/*
	* 1. Base station requests info from badge: OPCODE_REQUEST_BADGE_DUMP
	* 2. Badge respondes with OPCODE_BADGE_IDENTITY
	* 3. Badge responds with OPCODE_GAME_ID
	* 4. Badge responds with OPCODE_BADGE_RECORD_COUNT
	* 5. Badge responds with triplets of OPCODE_BADGE_UPLOAD_HIT_RECORD_BADGE_ID,
	*    OPCODE_BADGE_UPLOAD_HIT_RECORD_TIMESTAMP, and OPCODE_SET_BADGE_TEAM.
	* 6. Badge responds with 5 packets of OPCODE_USERNAME_DATA to transfer 10
	*    characters of the badge username.
	*/

	if (delay) {
		delay--;
		return;
	}

	/* This check is probably racy */
	if (((IRpacketOutNext+1) % MAXPACKETQUEUE) == IRpacketOutCurr) {
		return;
	}

	if (record_num == 0) {
		send_badge_identity_packet();
	} else if (record_num == 1) {
		send_game_id_packet(game_id);
	} else if (record_num == 2) {
		send_badge_record_count(nhits);
	} else if (record_num < nhits * 3 + 3) {
		int hit_index = (record_num - 3) / 3;
		switch ((record_num  - 3) % 3) {
		case 0:
			send_badge_upload_hit_record_badge_id(&hit_table[hit_index]);
			break;
		case 1:
			send_badge_upload_hit_record_timestamp(&hit_table[hit_index]);
			break;
		case 2:
			send_badge_upload_hit_record_team(&hit_table[hit_index]);
			break;
		}
	} else if (record_num < nhits * 3 + 3 + 5) {
		const int pkt_num = record_num - (nhits * 3 + 3);
		const int pkt_len = 2;
		unsigned short encoded;
		encoded = encode_username_substring(&username[pkt_num * 2], pkt_len);
#ifdef __linux__
		printf("username = '%s'\n", username);
		decode_username_fragment(encoded);
#endif
		send_badge_username_packet(encoded);
	} else {
		record_num = 0;
		game_state = GAME_PROCESS_BUTTON_PRESSES;
		return;
	}
	record_num++;
	delay = delay_count;
	return;
}

static void game_grant_powerup(void)
{
        int powerup = G_sysData.badgeId;

	if (!is_vendor_badge(G_sysData.badgeId))
		return;

        send_ir_packet(build_ir_packet(1, 1, BADGE_IR_GAME_ADDRESS, BADGE_IR_BROADCAST_ID,
                (OPCODE_VENDOR_POWER_UP << 12) | ((powerup + 1) & 0x1ff)));
	if (game_state == GAME_GRANT_POWERUP)
		game_state = GAME_MAIN_MENU;
}

static void game_grant_continuous_powerup(void)
{
	continuous_powerup_granting_enabled = 1;
	setup_main_menu();
	game_state = GAME_MAIN_MENU;
}

static void game_stop_powerups(void)
{
	continuous_powerup_granting_enabled = 0;
	setup_main_menu();
	game_state = GAME_MAIN_MENU;
}

static void clear_game_data(void)
{
	seconds_until_game_starts = NO_GAME_START_TIME;
	game_duration = -1;
	team = -1;
	game_variant = GAME_VARIANT_NONE;
	game_id = -1;
}

static int all_game_data_received(void)
{
	return (seconds_until_game_starts != NO_GAME_START_TIME &&
		game_duration != -1 &&
		team != -1 &&
		game_variant != GAME_VARIANT_NONE &&
		game_id != -1);
}

static void process_packet(unsigned int packet)
{
	unsigned int payload;
	unsigned char opcode;
	int v;

	if (packet == 32) /* Ignore spurious 32 that might come in. */
		return;

	payload = get_payload(packet);
	opcode = payload >> 12;
	switch (opcode) {
	case OPCODE_SET_GAME_START_TIME:
		if (!all_game_data_received()) /* Don't clear if we get a stray GAME START TIME packet */
			clear_game_data(); /* GAME START TIME is the first part of the game data... clear old game data */
		/* time is a 12 bit signed number */
		v = payload & 0x0fff;
		if (payload & 0x0800)
			v = v | 0xfffff800;
		seconds_until_game_starts = v;
		set_game_start_timestamp(seconds_until_game_starts);
		if (seconds_until_game_starts > 0)
			nhits = 0; /* don't reset counter if game already started? */
		screen_changed = 1;
		break;
	case OPCODE_SET_GAME_DURATION:
		/* time is 12 unsigned number */
		game_duration = payload & 0x0fff;
		screen_changed = 1;
		break;
	case OPCODE_HIT:
		process_hit(packet);
		break;
	case OPCODE_REQUEST_BADGE_DUMP:
		game_state = GAME_DUMP_DATA;
		break;
	case OPCODE_SET_BADGE_TEAM:
		team = payload & 0x07; /* TODO sanity check this better. */
		screen_changed = 1;
		break;
	case OPCODE_SET_GAME_VARIANT:
		game_variant = (payload & 0x0f) % ARRAYSIZE(game_type);
		screen_changed = 1;
		break;
	case OPCODE_GAME_ID:
		game_id = payload & 0x0fff;
		/* We happen to know this is the last bit of data for a game that the base
		 * station sends us. So at this time, we beep to indicate all the data for
		 * the game has been recieved. */
		if (all_game_data_received())
			setNote(80, 4000);
		else
			clear_game_data();
		break;
	case OPCODE_VENDOR_POWER_UP:
		process_vendor_powerup(packet);
		break;
	default:
		break;
	}
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

/* wrapper of flareled() to throttle calls to it to not disturb the pwm code too much */
static void lasertag_flareled(unsigned char r, unsigned char g, unsigned char b)
{
	static int throttler = 0;
	throttler++;
	if (throttler == 100) {
		flareled(r, g, b);
		throttler = 0;
	}
}

static void advance_time()
{
	int old_time = seconds_until_game_starts;
	int old_suppress = suppress_further_hits_until;
	static int already_sent_powerup = 0;
	static int timer = 0;

	timer++;

#ifdef __linux__
	struct timeval tv;

	gettimeofday(&tv, NULL);
	current_time = tv.tv_sec;
#else
	current_time = get_time();
#endif
	if (seconds_until_game_starts != NO_GAME_START_TIME && game_start_timestamp != NO_GAME_START_TIME)
		seconds_until_game_starts = game_start_timestamp - current_time;
	if (suppress_further_hits_until == current_time)
		setNote(70, 4000); /* Beep when deadtime expires */
	if (suppress_further_hits_until <= current_time) {
		suppress_further_hits_until = -1;
		switch (game_variant % ARRAYSIZE(game_type)) {
		case 0: /* free for all */
			lasertag_flareled(0, 0, 0);
			break;
		case 1: /* team battle */
		case 2: /* zombies */
		case 3: /* capture badge */
			if (team >= 0 && team < ARRAYSIZE(team_color)) /* Set LED flare to team colors */
				lasertag_flareled(team_color[team].r, team_color[team].g, team_color[team].b);
			else
				lasertag_flareled(0, 0, 0);
			break;
			/* TODO: still need to make the code to manipulate the team for zombies and capture badge */
		default:
			lasertag_flareled(0, 0, 0);
			break;
		}
	} else {
		/* blink led red for dead time */
#if __linux__
		if (timer & 0x08)
#else
		if (timer & 0x1000) /* this will need tuning */
#endif
			lasertag_flareled(255, 0, 0);
		else
#ifdef __linux__
			lasertag_flareled(128, 128, 128);
#else
			lasertag_flareled(0, 0, 0);
#endif

	}
	if (old_time != seconds_until_game_starts || old_suppress != suppress_further_hits_until)
		screen_changed = 1;
	if (old_time > 0 && seconds_until_game_starts <= 0)
		setNote(50, 4000); /* Beep upon game start */
	if ((current_time % AUTO_GRANT_POWERUP_INTERVAL) == 0) {
		if (continuous_powerup_granting_enabled && !already_sent_powerup) {
			game_grant_powerup();
			already_sent_powerup = 1;
		}
	} else {
		already_sent_powerup = 0;
	}
}

static void game_process_button_presses(void)
{
	check_for_incoming_packets();
	advance_time();
	if (BUTTON_PRESSED_AND_CONSUME) {
		button_pressed();
	} else if (UP_BTN_AND_CONSUME) {
		if (menu.menu_active)
			menu_change_current_selection(-1);
	} else if (DOWN_BTN_AND_CONSUME) {
		if (menu.menu_active)
			menu_change_current_selection(1);
	} else if (LEFT_BTN_AND_CONSUME) {
	} else if (RIGHT_BTN_AND_CONSUME) {
	}
	if (game_state == GAME_PROCESS_BUTTON_PRESSES && screen_changed)
		game_state = GAME_MAIN_MENU;
	return;
}

static void game_screen_render(void)
{
	game_state = GAME_PROCESS_BUTTON_PRESSES;
	if (!screen_changed)
		return;
	FbPushBuffer();
	screen_changed = 0;
}

static void game_shoot(void)
{
	unsigned int packet;
	unsigned short payload;

	if (current_time - game_start_timestamp < 0) { /* game has not started yet, do not shoot. */
		game_state = GAME_MAIN_MENU;
		return;
	}

	/* Player can only shoot if they are not currently dead. */
	if (suppress_further_hits_until == -1) {
		payload = (OPCODE_HIT << 12) | ((G_sysData.badgeId & 0x1ff) << 3) | (team & 0x07);
		packet = build_ir_packet(0, 1, BADGE_IR_GAME_ADDRESS, BADGE_IR_BROADCAST_ID, payload);
		send_ir_packet(packet);
	}

	game_state = GAME_MAIN_MENU;
}

int lasertag_cb(void)
{
	game_state_fn[game_state]();
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
	start_gtk(&argc, &argv, lasertag_cb, 30);
	return 0;
}

#endif
