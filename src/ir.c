#include <plib.h>
#include "colors.h"
#include "ir.h"
#include "flash.h"

unsigned short G_peerBadgeId=0;

unsigned char IRpacketInCurr = 0;
unsigned char IRpacketInNext = 0;
union IRpacket_u IRpacketsIn[MAXPACKETQUEUE];

unsigned char IRpacketOutCurr = 0;
unsigned char IRpacketOutNext = 0;
union IRpacket_u IRpacketsOut[MAXPACKETQUEUE];

/*
   IR stats
*/
unsigned int IR_inpkts = 0;
unsigned int IR_outpkts = 0;


// will be called by an interrupt 
// Set to a badge id if the badge got an init ping
volatile unsigned short pinged = 0 ;

// Set to a badge id if badge got a ping response
volatile unsigned short ping_responded = 0;

unsigned char QC_IR = 0;
extern volatile unsigned int last_packet;
extern volatile unsigned int last_packet_address;

/*
   Dont change the order of this without also changing the enum's in ir.h
*/
struct IRcallback_t IRcallbacks[] = {
	{ ir_name },
	{ ir_badgeid },
	{ ir_sekrits },
	{ ir_achievements },

	/* load/save settings */
	{ ir_led },
	{ ir_time },
	{ ir_date_YYMM },
	{ ir_date_DDAMPM },
	{ ir_screensaver },
	{ ir_backlight },

	/* special */
	{ ir_code },
	{ ir_forthcode },
        { ir_draw_unlockable },
	{ ir_asset },
        { ir_ping},

	{ ir_liveaudio },	/* stream play to piezo */
	{ ir_livetext },	/* stream text screen */
	{ ir_liveled },	/* stream rgb to LED */

	/* 
	   ================================
	         handled by apps
	   ================================
	*/
//	{ ir_udraw},
	{ ir_app0 },
	{ ir_app1 },
	{ ir_app2 },
	{ ir_app3 },

	{ ir_app4 },
	{ ir_app5 },
	{ ir_app6 },
	{ ir_app7 },

	{ ir_error },
	{ ir_lastaddress },
};

void IRqueueSend(union IRpacket_u pkt) 
{
    IR_outpkts++;
    /* make sure not full */
    if ( ((IRpacketOutNext+1) % MAXPACKETQUEUE) != IRpacketOutCurr) {
	IRpacketsOut[IRpacketOutNext].v = pkt.v;
	IRpacketOutNext++;
	IRpacketOutNext %= MAXPACKETQUEUE;
    }
}

void IRPair()
{
    union IRpacket_u pkt;
    
    // reset our peer and send out request to peer
    //G_peerBadgeId = 0;
    
    //Send ping
    pkt.p.command = IR_WRITE;
    pkt.p.address = IR_PING;
    // pkt.p.badgeId = G_sysData.badgeId; // ignore flash for now
    pkt.p.badgeId = 0;
    pkt.p.data    = PING_PAIR_REQUEST;
    IRqueueSend(pkt);
}

volatile union IRpacket_u G_hack={0};

extern int lasertag_screen_changed;

/* handler for IR recv in main thread */
void IRhandler() 
{
    /* 
       should we lock variable IRpacketInCurr incase of interupts? 
       No: this routine modifies InCurr, where the insert routine modfies InNext
     */

    /* curr == next == empty */
    if (IRpacketInCurr != IRpacketInNext) {
        IR_inpkts++;
	lasertag_screen_changed = 1;
	last_packet_address = IRpacketsIn[IRpacketInCurr].p.address;
        // if (IRpacketsIn[IRpacketInCurr].p.address < IR_LASTADRESS) /* basic sanity check before we call unknown handlers */
            IRcallbacks[ IRpacketsIn[IRpacketInCurr].p.address].handler( IRpacketsIn[IRpacketInCurr].p );

	memcpy(&last_packet, &IRpacketsIn[IRpacketInCurr].p, sizeof(last_packet));
        IRpacketInCurr++;
        IRpacketInCurr %= MAXPACKETQUEUE;
    }
}

void ir_name(struct IRpacket_t p)
{
}

void ir_badgeid(struct IRpacket_t p)
{
}

void ir_sekrits(struct IRpacket_t p)
{
}

void ir_achievements(struct IRpacket_t p)
{
}

void ir_led(struct IRpacket_t p)
{
    unsigned char r,g,b;

    r = UNPACKR(p.data);
    red(r);

    g = UNPACKG(p.data);
    green(g);

    b =  UNPACKB(p.data);
    blue(b);
}

void ir_time(struct IRpacket_t p)
{
}

void ir_date_YYMM(struct IRpacket_t p)
{
}

void ir_date_DDAMPM(struct IRpacket_t p)
{
}

void ir_screensaver(struct IRpacket_t p)
{
}

void ir_backlight(struct IRpacket_t p)
{
}

void ir_code(struct IRpacket_t p)
{
}

void ir_forthcode(struct IRpacket_t p)
{
}

void ir_draw_unlockable(struct IRpacket_t p)
{
        return;
}

void ir_asset(struct IRpacket_t p)
{
}


// This is the initial ping ( person initiating the pinging sent this)
void ir_ping(struct IRpacket_t p)
{
    //respond to ping
    union IRpacket_u pkt;
    
    pinged++;
    // Badge got a Ping from another badge
    if(p.data & PING_REQUEST)
    {
        // Get out the requester's ID
        //pinged = p.data & 0x1FF;
        //pinged = p.data;
        // If broadcast, just set to some high number
//        if(pinged == 0)
//            pinged = 1024;

        pkt.p.command = IR_WRITE;

        pkt.p.address = IR_PING;

        // Send along this badges ID
        pkt.p.badgeId = 0;//G_sysData.badgeId;
        pkt.p.data    = PING_RESPONSE | G_sysData.badgeId;

        IRqueueSend(pkt);
    }
    // badge got it's (or someone's) response to a PING_REQUEST
    else if(p.data & PING_RESPONSE)
    {
        //ping_responded = p.badgeId;
        ping_responded = p.data & 0x1FF;
    }
    // This badge got request to pair from another badge
    // Only care if badge doesn't yet have a peer
//    else if(p.data == PING_PAIR_REQUEST && !G_peerBadgeId)
//    {
//        // Get out the requester's ID
//        G_peerBadgeId = p.badgeId;
//
//        pkt.p.command = IR_WRITE;
//        pkt.p.address = IR_PING;
//
//        // Send along this badges ID
//        pkt.p.badgeId = G_sysData.badgeId;
//        pkt.p.data    = PING_PAIR_CONFIRM;
//
//        IRqueueSend(pkt);
//    }
//    // This badge got confirmation of pairing from another
//    // Only care if badge doesn't yet have a peer
//    else if(p.data == PING_PAIR_CONFIRM && !G_peerBadgeId)
//    {
//        // Get out the requester's ID
//        G_peerBadgeId = p.badgeId;
//    }
}

//void ir_ping_resp(struct IRpacket_t p)
//{
//    ping_responded = p.badgeId;
//   DEBUGSTRLVL(DNOISE, "ir_ping_resp");
//}

void ir_liveaudio(struct IRpacket_t p)
{
    unsigned short freq;
    unsigned short dur;
#ifdef XXX
    if (p.command == IR_READ)  {
	/* copy out audio we are playing */
	if (!G_IRsend) {
	   union IRpacket_u outp;

	   outp.p.command = IR_WRITE;
	   outp.p.address = IR_LIVEAUDIO;
	   outp.p.badgeId = p.data; /* badge that sent request */
	   outp.p.data = ((dur >> 8) << 12) | (freq & 0x0FFF) ;

           G_IRsendVal = outp.v;
           G_IRsend = 1;
	}
    }
    else
#endif
    {
	/* top 4 bits are for duration divided by 256 (>> 8) */
	dur  = ((p.data >> 12) & 0xF) << 8 ;
	/* bottom 12 bits are for freq */
	freq = p.data & 0xFFF;
	setNote(freq, dur);
    }

}

void ir_livetext(struct IRpacket_t p)
{
}

void ir_liveled(struct IRpacket_t p)
{
}

//extern void draw_ir_udraw(struct IRpacket_t p);//from udraw.c

//void ir_udraw(struct IRpacket_t p)
//{
//    draw_ir_udraw(p);
    //DEBUGSTRLVL(DNOISE, "ir_udraw");
//}

//QC APP
void ir_app0(struct IRpacket_t p)
{
    //DEBUGSTRLVL(DNOISE, "ir_app0");
    QC_IR = (unsigned char) p.data;
}

extern void lasertag_ir_packet_callback(struct IRpacket_t packet);

void ir_app1(struct IRpacket_t p)
{
	lasertag_ir_packet_callback(p);
}

void ir_app2(struct IRpacket_t p)
{
}

void ir_app3(struct IRpacket_t p)
{
}

void ir_app4(struct IRpacket_t p)
{
}

void ir_app5(struct IRpacket_t p)
{
}

void ir_app6(struct IRpacket_t p)
{
}

void ir_app7(struct IRpacket_t p)
{
}

void ir_error(struct IRpacket_t p)
{
}

void ir_lastaddress(struct IRpacket_t p)
{
}
