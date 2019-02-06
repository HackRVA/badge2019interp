#ifndef ir_h
#define ir_h

struct IRpacket_t {
       unsigned  data:16;
       unsigned  badgeId:9;
       unsigned  address:5;
       unsigned  command:1;
       unsigned  startBit:1;
} ;

union IRpacket_u {
    struct IRpacket_t p;
    unsigned int v;
};

#define PING_REQUEST      0x1000
#define PING_RESPONSE     0x2000

#define PING_PAIR_REQUEST 0x2
#define PING_PAIR_CONFIRM 0x3

/* input circular queue */
#define MAXPACKETQUEUE 8
extern unsigned char IRpacketInCurr ;
extern unsigned char IRpacketInNext ;
extern union IRpacket_u IRpacketsIn[];

/* output circular queue */
extern unsigned char IRpacketOutCurr ;
extern unsigned char IRpacketOutNext ;
extern union IRpacket_u IRpacketsOut[];

// Set to a badge id if the badge got an init ping
extern unsigned short pinged;

// Set to a badge id if badge got a ping response
extern unsigned short ping_responded;

struct IRcallback_t {
    void (*handler)(struct IRpacket_t p);
};

extern struct IRcallback_t IRcallbacks[];

enum {
   IR_READ,
   IR_WRITE
};

/*
   mapping to address array
   Dont change the order of this without also changing the functions in ir.c
*/
enum {
	IR_NAME,		/* lenght == 32? */
	IR_BADGEID,	/* send to ALL == 0b1111111111/1023/0x3F */
	IR_SEKRITS,	/* not sure, individual per game? */
	IR_ACHIEVEMENTS,   /* ? */

	/* load/save settings */
	IR_LED,	/* led brightness */

	IR_TIME,	/* time */

	IR_DATEYYMM,	/* date */
	IR_DATEDDAMPM,	/* date */

	IR_SCREENSAVER,	/* dunno brightness */
	IR_BACKLIGHT,	/* backlight brightness */

	/* special */
	IR_CODE,		/* code to jump to */
	IR_FORTHCODE,	/* forth code to run */
        IR_DRAW_UNLOCKABLE,
	IR_ASSET,		/* set current asset */
        IR_PING,

	IR_LIVEAUDIO,	/* stream play to piezo */
	IR_LIVETEXT,	/* stream text screen */
	IR_LIVELED,	/* stream rgb to LED */

	/* 
	   ================================
	         handled by apps
	   ================================
	*/
	IR_UDRAW,
	IR_APP0,
	IR_APP1,
	IR_APP2,
	IR_APP3,

	IR_APP4,
	IR_APP5,
	IR_APP6,
	IR_APP7,

	IR_ERROR,
	IR_LASTADRESS,
};

void IRhandler() ;
void IRqueueSend(union IRpacket_u pkt);
void IRPair();

void ir_name(struct IRpacket_t p);
void ir_badgeid(struct IRpacket_t p);
void ir_sekrits(struct IRpacket_t p);
void ir_achievements(struct IRpacket_t p);
void ir_led(struct IRpacket_t p);
void ir_time(struct IRpacket_t p);
void ir_date_YYMM(struct IRpacket_t p);
void ir_date_DDAMPM(struct IRpacket_t p);
void ir_screensaver(struct IRpacket_t p);
void ir_backlight(struct IRpacket_t p);
void ir_code(struct IRpacket_t p);
void ir_forthcode(struct IRpacket_t p);
void ir_draw_unlockable(struct IRpacket_t p);
void ir_asset(struct IRpacket_t p);
void ir_ping(struct IRpacket_t p);
void ir_liveaudio(struct IRpacket_t p);
void ir_livetext(struct IRpacket_t p);
void ir_liveled(struct IRpacket_t p);
void ir_udraw(struct IRpacket_t p);
void ir_app0(struct IRpacket_t p);
void ir_app1(struct IRpacket_t p);
void ir_app2(struct IRpacket_t p);
void ir_app3(struct IRpacket_t p);
void ir_app4(struct IRpacket_t p);
void ir_app5(struct IRpacket_t p);
void ir_app6(struct IRpacket_t p);
void ir_app7(struct IRpacket_t p);
void ir_error(struct IRpacket_t p);
void ir_lastaddress(struct IRpacket_t p);

#endif
