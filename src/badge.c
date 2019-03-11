#include <plib.h>
#include "colors.h"
#include "assetList.h"
#include "flash.h"
#include "adc.h"
#include "menu.h"
#include "interpreter.h"
#include "buttons.h"
#include "timer1_int.h"
#include "assets.h"
#include "build_bug_on.h"

#if BASE_STATION_BADGE_BUILD
#include "ir.h"
#include "usb_function_cdc.h" // for USBUSARTIsTxTrfReady(). (I guess... the docs and API suck.)
#endif

#include "USB/usb_config.h" // for buffer size/CDC_DATA_IN_EP_SIZE, CDC_DATA_OUT_EP_SIZE

extern char USB_In_Buffer[];
extern char USB_Out_Buffer[];
#if BASE_STATION_BADGE_BUILD
int USB_Out_Buffer_Len = 0; /* For base station, USB buffers are not asciiz strings */
#endif

/*
  persistant (flash) system data 
*/
struct sysData_t G_sysData;

const char hextab[16]={"0123456789ABCDEF"};

// in used in S6B33 samsung controller
extern unsigned char G_contrast1;

volatile unsigned char getUSBSuspendControl();
volatile unsigned char USBDeviceStateDETACHED();
volatile unsigned char USBDeviceStateATTACHED();
volatile unsigned char USBDeviceStatePOWERED();
volatile unsigned char USBDeviceStateDEFAULT();
volatile unsigned char USBDeviceStateADDRESS();
volatile unsigned char USBDeviceStateCONFIGURED();


    
/*
  main is actually in microchip.c
  however, this file contains the main code:

  UserInit()
  ProcessIO()

*/

unsigned int USBbufferSizeIn();
unsigned int USBbufferSizeOut();
volatile int mchipUSBnotReady();
volatile unsigned char USBtransferReady();

unsigned char  NextUSBOut=0;

void UserInit(void)
{
    CFGCONbits.JTAGEN = 0;

    ANSELA = 0x00;
    ANSELB = 0x00;
    ANSELC = 0x00;
    TRISA = 0;
    LATA = 0;
    TRISB = 0;
    LATB = 0;
    TRISC = 0;
    LATC = 0;


    /* RGB LED */
    TRISCbits.TRISC0 = 0;    /* output */
    LATCbits.LATC0 = 0;      /* red init low */
    CNPDCbits.CNPDC0 = 0;    /* pulldown == off */

    TRISCbits.TRISC1 = 0;    /* output */
    LATCbits.LATC1 = 0;      /* blue init low */
    CNPDCbits.CNPDC1 = 0;    /* pulldown == off */

    TRISBbits.TRISB3 = 0;    /* output */
    LATBbits.LATB3 = 0;      /* green init low */
    CNPDBbits.CNPDB3 = 0;    /* pulldown == off */


    LCDInitPins();
    LATCbits.LATC0 = 1;      /* RED */

    G_contrast1 -= 16;       /* PEB 2019 not sure if this is systemic or just one lcd */

    LCDReset();
    LATCbits.LATC1 = 1;      /* BLUE */

    //LCDBars();

    FbInit();
    FbClear();

 

    /* boot status */
    LATBbits.LATB3 = 1;      /* GREEN */

    /* backlight on. you will see nothing if it is off */
    LATCbits.LATC9 = 1;      
       
    
    /* 
	RB14 == main button
    */
    TRISBbits.TRISB14 = 1; // 2019 button == input
    CNPUBbits.CNPUB14 = 1; // 2019 pullup == on


    /*
        2019 D-PAD

        RC2 == RIGHT
        RB2 == TOP
        RB1 == BOTTOM
        RB0 == LEFT
    */

    TRISBbits.TRISB0 = 1; // 2019 button == input
    CNPUBbits.CNPUB0 = 1; // 2019 pullup == on

    TRISBbits.TRISB1 = 1; // 2019 button == input
    CNPUBbits.CNPUB1 = 1; // 2019 pullup == on

    TRISBbits.TRISB2 = 1; // 2019 button == input
    CNPUBbits.CNPUB2 = 1; // 2019 pullup == on

    TRISCbits.TRISC2 = 1; // 2019 button == input
    CNPUCbits.CNPUC2 = 1; // 2019 pullup == on


    /* 
	speaker pull down init

       RB4 speaker driver ch0
       RA8 speaker driver ch1

       1 1 active braking -> both halves of h-bridge grounded
       1 0 forward -> 
       0 1 reverse -> 
       0 0 off -> not grounded, coasting
    */
    TRISBbits.TRISB4 = 0; // 2019 output off
    CNPUBbits.CNPUB4 = 0; // 2019 pullup off

    TRISAbits.TRISA8 = 0; // 2019 output off
    CNPUAbits.CNPUA8 = 0; // 2019 pullup off

    /*
        2019 mic input
        RC3 / AN12
    */
    ANSELCbits.ANSC3 = 1; // is analog 
    TRISCbits.TRISC3 = 1; // mic input
    CNPUCbits.CNPUC3 = 0; // pullup off
    CNPDCbits.CNPDC3 = 0; // pulldown off

    /*
	flareleds

      LATC5 red
      LATC4 green
      LATA4 blue

      LATA9 ground/open drain
    */
#ifdef PAULSHACKEDBADGE
    TRISCbits.TRISC5 = 0; // output
    CNPUCbits.CNPUC5 = 0; // pullup off
    CNPDCbits.CNPDC5 = 0; // pulldown off
    LATCbits.LATC5 = 0;   // off initially

    TRISCbits.TRISC4 = 0; // output
    CNPUCbits.CNPUC4 = 0; // pullup off
    CNPDCbits.CNPDC4 = 0; // pulldown off
    LATCbits.LATC4 = 0;   // off initially

    TRISAbits.TRISA4 = 0; // output
    CNPUAbits.CNPUA4 = 0; // pullup off
    CNPDAbits.CNPDA4 = 0; // pulldown off
    LATAbits.LATA4 = 0;   // off initially

    TRISAbits.TRISA9 = 0; // output
    CNPUAbits.CNPUA9 = 0; // pullup off
    CNPDAbits.CNPDA9 = 0; // pulldown off
    ODCAbits.ODCA9 = 1;   // open drain ON makes this effectively a ground pin
    LATAbits.LATA9 = 0;   // draining
#endif


    /*
	IR led
    */
    TRISAbits.TRISA1 = 0; // output
    CNPUAbits.CNPUA1 = 0; // pullup off
    CNPDAbits.CNPDA1 = 0; // pulldown off
    LATAbits.LATA1 = 0;   /* off initially */

    /*
	leds all off/normal
    */
    LATCbits.LATC0 = 0;      /* RED */
    LATCbits.LATC1 = 0;      /* BLUE */
    LATBbits.LATB3 = 0;      /* GREEN */

    FbMove(10,10);
    FbWriteLine("testing");
    FbMove(10,20);
    FbPushBuffer();

    timerInit();
}

void decDump(unsigned int value, char *out) {
    int i;
    for (i=7; i>=0; i--) {
	out[i] = value % 10 + 48; 
	value /= 10; 
    }
    out[8]=0;
}

void hexDump(unsigned int value, char *out) {
    int i;
    for (i=7; i>=0; i--) 
       out[7-i] = hextab[(value >> (i*4)) & 0xF]; 
    out[8]=0;
}

/*
  usb calls only happen 100-ish times a second
  so buffer can get full before the usb call to drain
*/
static unsigned char lineOutBuffer[64], lineOutBufPtr=0;
/*
  callback to echo string from py stdout to USB
*/

void flushUSB();

void echoUSB(char *str) {
   int i,len;

   len = strlen(str);

   if (len > CDC_DATA_OUT_EP_SIZE) return;

   for (i=0; i<len; i++) {
	lineOutBuffer[lineOutBufPtr++] = str[i];
   }
   lineOutBuffer[lineOutBufPtr] = 0;

   flushUSB(); // flush every time
}

/*
   on line of text from input
*/
#define TEXTBUFFERSIZE 128
static unsigned char textBuffer[TEXTBUFFERSIZE], textBufPtr=0;

/*
   source code buffer
*/
#define SOURCEBUFFERSIZE 2048
char sourceBuffer[SOURCEBUFFERSIZE];

/*
  process one line of input
*/
void doLine()
{
    // call interpreter 
    if (strncmp(textBuffer,"run",3) == 0) {
	unsigned int r;

	/* prep for error ret */
	strcpy(&(lineOutBuffer[lineOutBufPtr]), "\r\n"); 
	lineOutBufPtr += 2;

	r = interpreterMain(sourceBuffer); 

	lineOutBufPtr = 0; 
	lineOutBuffer[lineOutBufPtr] = 0; 

	strcpy(&(lineOutBuffer[lineOutBufPtr]), "\r\nRet ");
	lineOutBufPtr += 6;

	decDump(r, &(lineOutBuffer[lineOutBufPtr]));
	lineOutBufPtr += 8; /* always converts 8 digits */

	strcpy(&(lineOutBuffer[lineOutBufPtr]), "  ");
	lineOutBufPtr += 2;

	hexDump(r, &(lineOutBuffer[lineOutBufPtr]));
	lineOutBufPtr += 8; /* always converts 8 digits */

        interpreterStats();

	lineOutBufPtr = 0; 
	lineOutBuffer[lineOutBufPtr] = 0; 
	//memset(sourceBuffer, 0, SOURCEBUFFERSIZE);
    }
    else if (strncmp(textBuffer,"reset",5) == 0) {
	interpreterInit0();
	setAlloc(0, 38, 6, 6, 50);
	interpreterAlloc();
    } 
    else if (strncmp(textBuffer,"new",3) == 0) {
	    memset(sourceBuffer, 0, SOURCEBUFFERSIZE);
    } 
    else if (strncmp(textBuffer,"list",4) == 0) {
	char *startS, *endS;

	startS = sourceBuffer;

	lineOutBufPtr = 0; 
	lineOutBuffer[lineOutBufPtr] = 0; 

	while (1) {
            endS = strchr(startS, '\n');

            if (endS == 0) break;

            strncpy(textBuffer, startS, endS-startS); //
            textBuffer[endS-startS] = '\0';
            echoUSB(textBuffer);
            echoUSB("\r\n");

            startS = endS+1;
	}
    } else {
	    strcat(sourceBuffer, textBuffer);
    }
    memset(textBuffer, 0, TEXTBUFFERSIZE);
    textBufPtr=0;
}


/*
  flush twice in case additional data send
*/
void check_usb_output(int *outp, int force)
{
    if (force || (*outp == (CDC_DATA_OUT_EP_SIZE-1))) {
	USB_Out_Buffer[*outp] = 0;
	*outp = 0; 
	flushUSB();
	flushUSB();
    }
}

unsigned char drbob=DRBOB;

const int *bindings[] = {
    (int *)&G_button_cnt, (int *)&G_up_button_cnt, (int *)&G_down_button_cnt, (int *)&G_left_button_cnt, (int *)&G_right_button_cnt,
    (int *)&(wclock.hour), (int *)&(wclock.min), (int *)&(wclock.sec),
    (int *)&G_sysData.badgeId, (int *)&G_sysData.name, (int *)&flashedBadgeId, (int *)&G_flashAddr,
    (int *)&drawLCD8, (int *)&drbob, 
};

#if BASE_STATION_BADGE_BUILD
/* relay_usb_buffer_to_ir batches up 4-byte packets and sends to ir. If the
 * total number of bytes is not evenly divisible by 4, the last remaining bytes
 * are not sent, but are buffered, and when more data is to be sent, these
 * remaining bytes will be sent first.
 *
 * The buffer is static, so this function is not re-entrant/thread safe.
 */
static void relay_usb_buffer_to_ir(unsigned char *from_usb_buffer, int length)
{
	int i;
	unsigned char *packet;
	static union IRpacket_u p;
	static int accum = 0;

	packet = (unsigned char *) &p.v;

	for (i = 0; i < length; i++) {
		packet[accum] = from_usb_buffer[i];
		accum++;
		if (accum == 4) {
			IRqueueSend(p);
			accum = 0;
		}
	}
}

void relay_ir_packet_to_usb_serial(union IRpacket_u p)
{
	int i, j;
	unsigned char *c = (unsigned char *) &p.v;
	BUILD_ASSERT((CDC_DATA_OUT_EP_SIZE % 4) == 0);

	for (i = 0; i < 4; i++)
		USB_Out_Buffer[USB_Out_Buffer_Len + i] = c[i];
	USB_Out_Buffer_Len += 4;
	if (USB_Out_Buffer_Len >= CDC_DATA_OUT_EP_SIZE)
		USB_Out_Buffer_Len = 0;
}
#endif

static unsigned char writeLOCK=0;
void ProcessIO(void)
{
    unsigned char nread=0;
    static int doInterpreter = 0;
    int i;

    /*
	this ProcessIO() is the badge main loop
	buttons are serviced only when the app finishes
	same with IR events and USB input/output
    */
    doButtons();
    IRhandler(); /* do any pending IR callbacks */
    dopersist(sizeof(bindings)/4, bindings); /* do after button and IR so we can intercept */
    menus();
    FbPushBuffer();

    if (mchipUSBnotReady()) return;


    if (writeLOCK == 0) {
	nread = getsUSBUSART(USB_In_Buffer, CDC_DATA_IN_EP_SIZE);
    }

    if(nread > 0) {
#if BASE_STATION_BADGE_BUILD
        relay_usb_buffer_to_ir(USB_In_Buffer, nread);
#else
	   int i, outp=0;

	   for (i=0; i < nread; i++) {
		if (USB_In_Buffer[i] == 13) USB_In_Buffer[i] = 10;

		if ((USB_In_Buffer[i] == '') | (USB_In_Buffer[i] == '')) {
		   if (textBufPtr > 0) textBufPtr--;

		   USB_Out_Buffer[outp++] = '';
		   check_usb_output(&outp, 0);

		   USB_Out_Buffer[outp++] = ' ';
		   check_usb_output(&outp, 0);

		   USB_Out_Buffer[outp++] = '';
		   check_usb_output(&outp, 0);
		}
		else {
		   textBuffer[textBufPtr++] = USB_In_Buffer[i]; // used for python
		   textBuffer[textBufPtr] = 0; 

		   //if ((USB_In_Buffer[i] == 10) | (USB_In_Buffer[i] == 13)) {
		   if (USB_In_Buffer[i] == 10) {
			USB_Out_Buffer[outp++] = 13; // insert before the char
		   	check_usb_output(&outp, 0);

			USB_Out_Buffer[outp++] = 10; // insert before the char

		   	check_usb_output(&outp, 1); // force out

			doLine();
		   }
		   else {
			USB_Out_Buffer[outp++] = USB_In_Buffer[i];
		   	check_usb_output(&outp, 0);
		   }
		}
	    }
	    USB_Out_Buffer[outp] = 0; // null in case
	nread = 0;
#endif
    }

    flushUSB();
}

void flushUSB()
{
    while (!USBtransferReady()) CDCTxService();

    //if (USBtransferReady()) {
    {
#if BASE_STATION_BADGE_BUILD
	if (USB_Out_Buffer_Len <= 0)
		return;
	if (USBUSARTIsTxTrfReady()) {
		putUSBUSART(USB_Out_Buffer, USB_Out_Buffer_Len);
		USB_Out_Buffer_Len = 0;
	}
#else
	int len;

	if (writeLOCK) {
	   USB_Out_Buffer[0] = 0;
	   writeLOCK = 0;
	} 

	// jam interpreter line buffer into buffer since usb is done
	if (lineOutBufPtr != 0) {
	   strncpy(USB_Out_Buffer, lineOutBuffer, lineOutBufPtr);
	   USB_Out_Buffer[lineOutBufPtr] = 0;
	   lineOutBufPtr = 0;
	   lineOutBuffer[lineOutBufPtr] = 0;
	}

	len = strlen(USB_Out_Buffer);
	if (len != 0) {
	   putUSBUSART(USB_Out_Buffer, len);
	   writeLOCK = 1; // dont touch until USB done
	}
#endif
    }
}
