#include <plib.h>
#include "colors.h"
#include "assetList.h"
#include "flash.h"

#include "USB/usb_config.h" // for buffer size/CDC_DATA_IN_EP_SIZE

extern char USB_In_Buffer[];
extern char USB_Out_Buffer[];

/*
  persistant (flash) system data 
*/
struct sysData_t G_sysData;

const char hextab[]={"0123456789ABCDEF"};

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

    LCDBars();

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
	leds normal
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

void LCDprint(char *str,int len) {
   FbWriteString(str, len);
}

const char htab[]={"0123456789ABCDEF"};

void hexDump(int value, char *out) {
    *out = htab[(value >>  28) & 0xF]; out++;
    *out = htab[(value >>  24) & 0xF]; out++;
    *out = htab[(value >>  20) & 0xF]; out++;
    *out = htab[(value >>  16) & 0xF]; out++;
    *out = htab[(value >>  12) & 0xF]; out++;
    *out = htab[(value >>   8) & 0xF]; out++;
    *out = htab[(value >>   4) & 0xF]; out++;
    *out = htab[(value       ) & 0xF]; out++;
}

/*
  usb calls only happen 100-ish times a second
  so buffer can get full before the usb call to drain
*/
static unsigned char lineOutBuffer[64], lineOutBufPtr=0;
/*
  callback to echo string from py stdout to USB
*/
void echoUSB(char *str,int len) {
   int i;

   // can use USB_Out_Buffer since it may be locked in a host xfer
   if ((lineOutBufPtr + len) > CDC_DATA_OUT_EP_SIZE) return;

   for (i=0; i<len; i++) {
	if (str[i] == '\n')
	   lineOutBuffer[lineOutBufPtr++] = '\r';

	lineOutBuffer[lineOutBufPtr++] = str[i];
   }
   lineOutBuffer[lineOutBufPtr] = 0;
}

static char progbuffer[1024]={0}; 

// controls USB heartbeat blink
static unsigned char debugBlink=1;

void menus();
void ProcessIO(void)
{
    unsigned char nread=0;
    static unsigned char writeLOCK=0;
    static unsigned char textBuffer[128], textBufPtr=0;
    static int doInterpreter = 0;
    int i;

    if (mchipUSBnotReady()) return;

    doButtons();
    menus();
    //FbSwapBuffers();
    FbPushBuffer();
    IRhandler(); /* do any pending IR callbacks */


    if (writeLOCK == 0) {
	nread = getsUSBUSART(USB_In_Buffer, CDC_DATA_IN_EP_SIZE-1);
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
		if ((USB_In_Buffer[i] == '') | (USB_In_Buffer[i] == '')) {
		   if (textBufPtr > 0) textBufPtr--;

		   USB_Out_Buffer[outp++] = '';
		   USB_Out_Buffer[outp++] = ' ';
		   USB_Out_Buffer[outp++] = '';
		}
		else {
		   textBuffer[textBufPtr++] = USB_In_Buffer[i]; // used for python
		   textBuffer[textBufPtr] = 0; 

		   if ((USB_In_Buffer[i] == 10) | (USB_In_Buffer[i] == 13)) {
			USB_Out_Buffer[outp++] = 10; // insert before the char
			USB_Out_Buffer[outp++] = 13; // insert before the char
		        USB_Out_Buffer[outp] = 0;

			doInterpreter = 1;
			continue;
		   }
		   else {
		      USB_Out_Buffer[outp++] = USB_In_Buffer[i];
		      USB_Out_Buffer[outp] = 0;
		   }
		}
	   }

	   USB_Out_Buffer[outp] = 0; // null in case
	   USB_In_Buffer[0] = 0;
	   memset(USB_In_Buffer, 0, CDC_DATA_IN_EP_SIZE);
	}

	nread = 0;
    }


    if (USBtransferReady()) {
	int nextWrite;

	if (writeLOCK) {
	   USB_Out_Buffer[0] = 0;
	   writeLOCK = 0;
	} 

	// jam python line buffer into buffer since usb is done
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

    // do this after USB input, output processed
    if (doInterpreter) {
	// call interpreter 
	if (strncmp(textBuffer,"run",3) == 0) {
	   int r;

	   r = interpreter_main(progbuffer); 

	   strcpy(&(lineOutBuffer[lineOutBufPtr]), "interp return ");
	   lineOutBufPtr += 14;

	   hexDump(r, &(lineOutBuffer[lineOutBufPtr]));
	   lineOutBufPtr += 8;
	   lineOutBuffer[lineOutBufPtr++]=10;
	   lineOutBuffer[lineOutBufPtr++]=13;
	   lineOutBuffer[lineOutBufPtr++]=0;

	   memset(progbuffer, 0, 1024);
	}
	else {
	   strcat(progbuffer, textBuffer);
	}
	memset(textBuffer, 0, 128);
	textBufPtr=0;

	doInterpreter = 0;
    }

    CDCTxService();
}


/*
   USB status leds colors
*/
#define mInitAllLEDs()      LATB &= 0x0; TRISB &= 0x0;

#define mLED_1              LATCbits.LATC0 /* red */
#define mLED_2              LATCbits.LATC1  /* blue */
#define mLED_3              LATBbits.LATB3 /* green */
    
#define mGetLED_1()         mLED_1
#define mGetLED_2()         mLED_2
#define mGetLED_3()         mLED_3

#define mLED_1_On()         mLED_1 = 1;
#define mLED_2_On()         mLED_2 = 1;
#define mLED_3_On()         mLED_3 = 1;
    
#define mLED_1_Off()        mLED_1 = 0;
#define mLED_2_Off()        mLED_2 = 0;
#define mLED_3_Off()        mLED_3 = 0;
    
#define mLED_1_Toggle()     mLED_1 = !mLED_1;
 
void BlinkUSBStatus(void)
{
    static int led_count=0;

    if(led_count == 0) led_count = 100000U;
    led_count--;

    #define mLED_Both_Off()         {mLED_1_Off();mLED_2_Off();}
    #define mLED_Both_On()          {mLED_1_On();mLED_2_On();}
    #define mLED_Only_1_On()        {mLED_1_On();mLED_2_Off();}
    #define mLED_Only_2_On()        {mLED_1_Off();mLED_2_On();}

    if(getUSBSuspendControl() == 1) {
        if(led_count==0) {
            mLED_1_On();
            mLED_2_On();
        }
    }
    else {
	if (USBDeviceStateDETACHED())
        {
            mLED_Both_Off();
        }
        else if (USBDeviceStateATTACHED())
        {
            mLED_Both_On();
        }
        else if (USBDeviceStatePOWERED())
        {
            mLED_Only_1_On();
        }
        else if (USBDeviceStateDEFAULT())
        {
            mLED_Only_2_On();
        }
        else if (USBDeviceStateADDRESS())
        {
            if(led_count == 0) {
                mLED_1_Toggle();
                mLED_2_Off();
            }
        }
        else if (USBDeviceStateCONFIGURED()) {
            if(debugBlink && (led_count==0)) {
                if (mGetLED_1()) {
			mLED_1_Off();
		}
		else {
			mLED_1_On();
		}

                if (mGetLED_2()) {
			mLED_2_Off();
		}
		else {
			mLED_2_On();
		}

                if (mGetLED_3())  {
			mLED_3_Off();
		}
		else {
			mLED_3_On();
		}
            }
        }
    }
}

