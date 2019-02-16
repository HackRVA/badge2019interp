#include <plib.h>
#include "colors.h"
#include "assetList.h"
#include "flash.h"
#include "adc.h"
#include "menu.h"

#include "USB/usb_config.h" // for buffer size/CDC_DATA_IN_EP_SIZE

extern char USB_In_Buffer[];
extern char USB_Out_Buffer[];

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
    ANSELCbits.ANSC3 = 1;
    TRISCbits.TRISC3 = 1; // mic is an analog input
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
    ADC_init(AN_MIC, HZ_150); // init to sampling mic very slowly

    led(100,0,100);

}

void LCDprint(char *str,int len) {
   FbWriteString(str, len);
}

const char htab[]={"0123456789ABCDEF"};

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
       out[i] = htab[(value >> (i*4)) & 0xF]; 
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
   // can use USB_Out_Buffer since it may be locked in a host xfer
   if ((lineOutBufPtr + len) > CDC_DATA_OUT_EP_SIZE) {
	flushUSB();
   }

   /* separate output strings */
   //lineOutBuffer[lineOutBufPtr++] = '\r';
   //lineOutBuffer[lineOutBufPtr++] = '\n';
   //lineOutBuffer[lineOutBufPtr++] = ' ';

   for (i=0; i<len; i++) {
	lineOutBuffer[lineOutBufPtr++] = str[i];
   }
   lineOutBuffer[lineOutBufPtr] = 0;
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

void interp_stats();
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

	r = interpreter_main(sourceBuffer); 

	strcpy(&(lineOutBuffer[lineOutBufPtr]), "\r\nR ");
	lineOutBufPtr += 4;

	decDump(r, &(lineOutBuffer[lineOutBufPtr]));
	lineOutBufPtr += 8; /* always converts 8 digits */
//	lineOutBuffer[lineOutBufPtr++]=13;
//	lineOutBuffer[lineOutBufPtr++]=10;
//	lineOutBuffer[lineOutBufPtr]=0;

        interp_stats();

	memset(sourceBuffer, 0, SOURCEBUFFERSIZE);
    }
    else {
	strcat(sourceBuffer, textBuffer);
    }
    memset(textBuffer, 0, TEXTBUFFERSIZE);
    textBufPtr=0;
}

static unsigned char writeLOCK=0;
void ProcessIO(void)
{
    unsigned char nread=0;
    static int doInterpreter = 0;
    int i;

    if (mchipUSBnotReady()) return;

    /*
	this ProcessIO() is the badge main loop
	buttons are serviced only when the app finishes
	same with IR events and USB input/output
    */
    doButtons();
    IRhandler(); /* do any pending IR callbacks */
    menus();
    FbPushBuffer();


    if (writeLOCK == 0) {
	nread = getsUSBUSART(USB_In_Buffer, CDC_DATA_IN_EP_SIZE);
    }

    if(nread > 0) {
	// LCD contrast
#ifdef BADFORINTERP
	if ((USB_In_Buffer[0] == '-') || (USB_In_Buffer[0] == '+')) {
	   // for S6B33 G_contrast1
	   if (USB_In_Buffer[0] == '-') G_contrast1 -= 4;
	   if (USB_In_Buffer[0] == '+') G_contrast1 += 4;

	   LCDReset();

	   USB_In_Buffer[0] = 0;
	}
#endif

	if (USB_In_Buffer[0] != 0) {
	   int i, outp=0;

	   for (i=0; i < nread; i++) {
		if (USB_In_Buffer[i] == 13) USB_In_Buffer[i] = 10;

		if ((USB_In_Buffer[i] == '') | (USB_In_Buffer[i] == '')) {
		   if (textBufPtr > 0) textBufPtr--;

		   USB_Out_Buffer[outp++] = '';
		   if (outp==CDC_DATA_OUT_EP_SIZE) outp--;
		   USB_Out_Buffer[outp++] = ' ';
		   if (outp==CDC_DATA_OUT_EP_SIZE) outp--;
		   USB_Out_Buffer[outp++] = '';
		   if (outp==CDC_DATA_OUT_EP_SIZE) outp--;
		}
		else {
		   textBuffer[textBufPtr++] = USB_In_Buffer[i]; // used for python
		   textBuffer[textBufPtr] = 0; 

		   //if ((USB_In_Buffer[i] == 10) | (USB_In_Buffer[i] == 13)) {
		   if (USB_In_Buffer[i] == 10) {
			USB_Out_Buffer[outp++] = 13; // insert before the char
		        if (outp==CDC_DATA_OUT_EP_SIZE) outp--;
			USB_Out_Buffer[outp++] = 10; // insert before the char
		        if (outp==CDC_DATA_OUT_EP_SIZE) outp--;
		        USB_Out_Buffer[outp] = 0;

			doLine();
		   }
		   else {
		      USB_Out_Buffer[outp++] = USB_In_Buffer[i];
		      if (outp==CDC_DATA_OUT_EP_SIZE) outp--;
		      USB_Out_Buffer[outp] = 0;
		   }
		}
	   }
	   USB_Out_Buffer[outp] = 0; // null in case
	   //USB_In_Buffer[0] = 0;
	}
	nread = 0;
    }

#define FLUSHUSB
#ifdef FLUSHUSB
    flushUSB();
#else
    if (USBtransferReady()) {
	int nextWrite;

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

	nextWrite = strlen(USB_Out_Buffer);
	if (nextWrite != 0) {
	   putUSBUSART(USB_Out_Buffer, nextWrite);
	   writeLOCK = 1; // dont touch until USB done
	}
    }
    CDCTxService();
#endif
}

#ifdef FLUSHUSB
void flushUSB()
{
    if (USBtransferReady()) {
	int nextWrite;

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

	nextWrite = strlen(USB_Out_Buffer);
	if (nextWrite != 0) {
	   putUSBUSART(USB_Out_Buffer, nextWrite);
	   writeLOCK = 1; // dont touch until USB done
	}
    }
    CDCTxService();
}
#endif