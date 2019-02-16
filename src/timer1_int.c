#include <plib.h>
#include "flash.h"
#include "ir.h"
#include "adc.h"

/*
    38khz IR timer code and interupt code
    120 fps timer interupt code
    LED PWM
    backlight PWM

    Author: Paul Bruggeman
    paul@killercats.com
    4/2015
*/

/*
    2018 reworked high -> low priority

    priority 6 = external int 4. IR recv. started
    priority 5 = timer2 IR send/recv
    priority 4 = ADC analog/digital converter
    priority 3 = timer4 audio PWM
    priority 2 = timer3 LED PWM

*/

// timer4 is PWM and audio and has priority 1 (low)
// timer2 is IR send and has priority 2
// ADC priority 5
// external interrupt 4 has priority 6  (high)



#define SYS_FREQ 		(40000000L)

#define IR_TOGGLES		38000
#define T2_TICK       		(SYS_FREQ/IR_TOGGLES)

//#define LED_TOGGLES		38000
#define LED_TOGGLES		40000
#define T3_TICK       		(SYS_FREQ/LED_TOGGLES)

/* audio timer4*/
#define AUDIO_TOGGLES		38000
#define T4_TICK       		(SYS_FREQ/AUDIO_TOGGLES)

void doLED_PWM();
void flarePWM();

void timerInit(void)
{
    // IR input pin input uses priority 4 
    // and flags receive timer2 to start

    // timer2 = IR send
    OpenTimer2(T2_ON | T2_SOURCE_INT, T2_TICK);
    ConfigIntTimer2(T2_INT_ON | T2_INT_PRIOR_5);

    // timer3 = led
    OpenTimer3(T3_ON | T3_SOURCE_INT, T3_TICK);
    ConfigIntTimer3(T3_INT_ON | T3_INT_PRIOR_2);

    // timer4 = audio
    OpenTimer4(T4_ON | T4_SOURCE_INT, T4_TICK);
    ConfigIntTimer4(T4_INT_ON | T4_INT_PRIOR_3);

    // enable multi-vector interrupts
    INTEnableSystemMultiVectoredInt();

#ifdef PRE2019
    IEC0bits.INT1IE=0; // 2015 disable this interrupt

    TRISBbits.TRISB0 = 1; // 2015 IR IN
    CNPDBbits.CNPDB0 = 0;  // pulldown off
    CNPUBbits.CNPUB0 = 0;  // pullup off

    TRISBbits.TRISB13 = 0; // 2015 IR == OUTPUT
    CNPDBbits.CNPDB13 = 0;  // pulldown off
    CNPUBbits.CNPUB13 = 1;  // pullup on == IR off since driving transistor inverts val

    SYSKEY = 0x0;
    SYSKEY = 0xAA996655;
    SYSKEY = 0x556699AA;

    CFGCONbits.IOLOCK = 0; // unlock configuration
    INT1Rbits.INT1R = 0b0010; // remap RPB0 to INT1
    CFGCONbits.IOLOCK = 1; // relock configuration

    SYSKEY = 0x0;

    INTCONbits.INT1EP=0; // edge polarity
    IPC1bits.INT1IP=6; // interrupt priority
    IPC1bits.INT1IS=0; // interrupt sub priority
    IEC0bits.INT1IE=1; // enable this interrupt

#else
    IEC0bits.INT4IE=0; // temp disable this interrupt

    TRISBbits.TRISB15 = 1; // 2019 IR IN
    CNPDBbits.CNPDB15 = 0;  // pulldown off
    CNPUBbits.CNPUB15 = 1;  // pullup on hardware ** 2019

    TRISAbits.TRISA1 = 0; // 2019 IR == OUTPUT
    CNPDAbits.CNPDA1 = 0;  // pulldown off
    CNPUAbits.CNPUA1 = 1;  // pullup on == IR off since driving transistor inverts val

    SYSKEY = 0x0;
    SYSKEY = 0xAA996655;
    SYSKEY = 0x556699AA;

    CFGCONbits.IOLOCK = 0; // unlock configuration
    INT4Rbits.INT4R = 0b0011; // remap pin RPB15 to INT4
    CFGCONbits.IOLOCK = 1; // relock configuration


    SYSKEY = 0x0;

    INTCONbits.INT4EP=0; // int on fall edge
    IPC4bits.INT4IP=6; // interrupt priority
    IPC4bits.INT4IS=0; // interrupt sub priority
    IEC0bits.INT4IE=1; // enable this interrupt
#endif

    IEC0bits.T2IE=1; // also enable timer2 interupt
}

// EXT4 int enabled when IR recv is 0 otherwise 1+
unsigned int G_IRsendVal = 0;
unsigned int G_IRrecvVal = 0;

unsigned int G_IRrecv = 0;
unsigned int G_IRsend = 0;

unsigned char G_IRerror = 0;

unsigned char G_bitCnt = 0;
unsigned char G_firstHalf = 0;
unsigned char G_lastHalf = 0;
unsigned char G_halfCount = 0;


/* 
  IR send/receive timer code

  if (G_IRrecv) then do receive (triggered by ext int)
  else if (G_IRsend) do send

  this code is based on RC5 timing from badge 2013
  almost certainly doesnt work for RC5 with a 38khz receiver

  min 6 cycle burst
  with burst (6-35 cycles) min gap between = 10 cycle
  max 2000 short burst/second
  with burst > 70 cycle needs game of 1.2 * burst len
*/
void __ISR(_TIMER_2_VECTOR, IPL5SOFT) Timer2Handler(void)
{
   void do_audio();
   void do_leds();
   static unsigned int sendOne = 0;
   static unsigned int sendZero = 0;
   static unsigned char lowHalf = 1;
   static unsigned char highHalf = 1;

   // each timer interrupt is 1/38khz
   if (G_IRrecv == 1) {
	if (G_bitCnt > 31) {
	    /* make sure queue not full */
	    if (((IRpacketInNext+1) % MAXPACKETQUEUE) != IRpacketInCurr) {
		IRpacketsIn[IRpacketInNext].v = G_IRrecvVal;
		/* overwrite if error */
		if (G_IRerror) {
		   IRpacketsIn[IRpacketInNext].p.address = IR_ERROR;
		   IRpacketsIn[IRpacketInNext].p.data = G_IRerror;
		   G_IRerror = 0;
		}

		/* if packet is not for us don't keep it */
		if ((IRpacketsIn[IRpacketInNext].p.badgeId == 0) 
		 | (IRpacketsIn[IRpacketInNext].p.badgeId == G_sysData.badgeId)) {
		    IRpacketInNext++;
		    IRpacketInNext %= MAXPACKETQUEUE;
		}
	    }
	    G_IRrecv = 0; // flag new value available
	    G_bitCnt = 0;
	    G_halfCount = 0;
	}
	else {
	    G_halfCount++;
	    // 32 interrupts for each half of bit send
	    // for 64 total per bit

	    if (G_halfCount == 16) G_firstHalf = !PORTBbits.RB15; 
	    if (G_halfCount == 48) G_lastHalf = !PORTBbits.RB15;

	    if (G_halfCount > 63) {
		G_IRrecvVal <<= 1 ;
		//LATBbits.LATB0 = G_lastHalf; // dbg LED output

		if ((G_firstHalf + G_lastHalf) != 1) G_IRerror++; /* increment errors. Error if not all manchester low->high, high->low transitions */
		G_IRrecvVal |= G_lastHalf;  

		G_bitCnt++;
		G_halfCount = 0;
	   }
	}
	// clear the interrupt flag
	mT2ClearIntFlag();

	return;
   }

   /* if we get this far then we are not in a receive */
   if (G_IRsend) {
        // 3 sections for IR send:
        // 1) init and looping vars
        // 2) send a zero
        // 3) send a one

        // break byte into bits to send
	// init, and looping. is the one or zero is done?
	if ((sendOne == 0) && (sendZero == 0)) {
	   if (G_bitCnt < 32) {
		// high order bit first
		if (G_bitCnt == 0) 
		   sendOne = 1; // send start bit
		else
		   sendOne = (G_IRsendVal & ((unsigned long long)0b10000000000000000000000000000000 >> G_bitCnt));

		sendZero = (sendOne == 0);
		G_bitCnt++;
	   }
	   else {
		G_IRsend = 0;
		G_bitCnt = 0;

	   }
	}


      // ;    A "1" in Manchester goes from Low-High over the bit period
      // ;    cycle. During the pulsing period, the carrier frequency should
      // ;    be 38kHz, and the duty cycle of the carrier should be about 1/4.
      // ;------------------------------------------------------------------

      //      ; LOW HALF 889us
      // 32 cycles off

      //      ; HIGH HALF (889us)
      //      ; Toggle 7us on, 21us off for 38kHz for the duration
      //      ; Pulsing 7 on, 21 off yields a 1/4 time duty cycle for carrier.
      //      ; Has 32 pulses of these periods 32*28us
      //      ;

      // each timer interrupt is 1/38khz
      if (sendOne) {
	 G_halfCount++;

	 if (lowHalf) {
	    // this is off for 32 counts

	    LATAbits.LATA1 = 0;

	    if (G_halfCount > 31) {
	      lowHalf = 0;
	      G_halfCount = 0;
	    }
	 }
	 else { 
	   // this is on for 7us (1/4 duty cycle) in each 
	   // 32 count 1/38khz interrupt

	   // high half
	   // 4 assignments is about 1us
	   // so 7us is about 4 * 7 assignments

// RA1
	   LATAbits.LATA1 = 1;
	   LATAbits.LATA1 = 1;
	   LATAbits.LATA1 = 1;
	   LATAbits.LATA1 = 1;

	   LATAbits.LATA1 = 1;
	   LATAbits.LATA1 = 1;
	   LATAbits.LATA1 = 1;
	   LATAbits.LATA1 = 1;

	   LATAbits.LATA1 = 1;
	   LATAbits.LATA1 = 1;
	   LATAbits.LATA1 = 1;
	   LATAbits.LATA1 = 1;

	   LATAbits.LATA1 = 1;
	   LATAbits.LATA1 = 1;
	   LATAbits.LATA1 = 1;
	   LATAbits.LATA1 = 1;

	   LATAbits.LATA1 = 1;
	   LATAbits.LATA1 = 1;
	   LATAbits.LATA1 = 1;
	   LATAbits.LATA1 = 1;

	   LATAbits.LATA1 = 1;
	   LATAbits.LATA1 = 1;
	   LATAbits.LATA1 = 1;
	   LATAbits.LATA1 = 1;

	   LATAbits.LATA1 = 1;
	   LATAbits.LATA1 = 1;
	   LATAbits.LATA1 = 1;
	   LATAbits.LATA1 = 1;

	   LATAbits.LATA1 = 0;

	    if (G_halfCount > 31) {
	      lowHalf = 1;
	      G_halfCount = 0;
	      sendOne = 0;
	    }
	 }
      }

      // ;------------------------------------------------------------------
      // ;  SendZero
      // ;
      // ;            A "0" in Manchester goes from High-Low over the bit period.
      // ;    The high period is a series of pulses of duty cycle 1/4 at a
      // ;    frequency of 36kHz.  This implementation yields 35.71kHz.
      // ;------------------------------------------------------------------
      // 
      //      ; HIGH HALF (889us)
      //      ; Toggle 7us on, 21us off for 35.7kHz (~36kHz) for the duration
      //      ; Pulsing 7 on, 21 off yields a 1/4 time duty cycle for carrier.
      //      ; Has 32 pulses of these periods 32*28us = 896us (~889us)
      //
      if (sendZero) {
	 G_halfCount++;
	 if (highHalf) {
	   // high half
	   // 4 assignments is about 1us
	   // so 7us is about 4 * 7 assignments

	   LATAbits.LATA1 = 1;
	   LATAbits.LATA1 = 1;
	   LATAbits.LATA1 = 1;
	   LATAbits.LATA1 = 1;

	   LATAbits.LATA1 = 1;
	   LATAbits.LATA1 = 1;
	   LATAbits.LATA1 = 1;
	   LATAbits.LATA1 = 1;

	   LATAbits.LATA1 = 1;
	   LATAbits.LATA1 = 1;
	   LATAbits.LATA1 = 1;
	   LATAbits.LATA1 = 1;

	   LATAbits.LATA1 = 1;
	   LATAbits.LATA1 = 1;
	   LATAbits.LATA1 = 1;
	   LATAbits.LATA1 = 1;

	   LATAbits.LATA1 = 1;
	   LATAbits.LATA1 = 1;
	   LATAbits.LATA1 = 1;
	   LATAbits.LATA1 = 1;

	   LATAbits.LATA1 = 1;
	   LATAbits.LATA1 = 1;
	   LATAbits.LATA1 = 1;
	   LATAbits.LATA1 = 1;

	   LATAbits.LATA1 = 1;
	   LATAbits.LATA1 = 1;
	   LATAbits.LATA1 = 1;
	   LATAbits.LATA1 = 1;

	   LATAbits.LATA1 = 0;

	   if (G_halfCount > 31) {
	     highHalf = 0;
	     G_halfCount = 0;
	   }
	 }
	 else {
	   //      ; LOW HALF (889us = 889 instr cycles)

	   LATAbits.LATA1 = 0;

	   if (G_halfCount > 31) {
	      highHalf = 1;
	      G_halfCount = 0;
	      sendZero = 0;
	   }
	 }
      }
   }
   else { /* not currently sending, check if IR queue is empty */
	if (IRpacketOutNext != IRpacketOutCurr) {
	    G_IRsendVal = IRpacketsOut[IRpacketOutCurr].v;
	    G_IRsend = 1;
	    IRpacketOutCurr++;
	    IRpacketOutCurr %= MAXPACKETQUEUE;
	}
   }

   // clear the interrupt flag
   mT2ClearIntFlag();

   return;
}

#ifdef PRE2019
// external IR receive: interrupt level 6 highest
// input changed on RB0
void __ISR( _EXTERNAL_1_VECTOR, IPL6SOFT) Int1Interrupt(void)
{ 
   // if not sending, signal in receive
   if ((G_IRsend == 0) & (G_IRrecv == 0)) {
	G_IRrecv = 1;

	// firstHalf was zero, this is one, so bit is 1
	G_firstHalf = 0; // inverted from what it was
	G_halfCount = 32;
	G_IRrecvVal = 0;

   }
   IFS0bits.INT1IF = 0;
}
#else
// external IR receive: interrupt level 6 highest
// input changed on RB15
void __ISR( _EXTERNAL_4_VECTOR, IPL6SOFT) Int4Interrupt(void)
{ 
   // if not sending, signal in receive
   if ((G_IRsend == 0) & (G_IRrecv == 0)) {
	G_IRrecv = 1;

	// firstHalf was zero, this is one, so bit is 1
	G_firstHalf = 0; // inverted from what it was
	G_halfCount = 32;
	G_IRrecvVal = 0;

   }
   IFS0bits.INT4IF = 0;
}
#endif

// audio higher than LED
void __ISR(_TIMER_4_VECTOR, IPL3SOFT) Timer4Handler(void)
{
   doAudio();
   mT4ClearIntFlag(); // clear the interrupt flag
}

// LED PWM lowest
void __ISR(_TIMER_3_VECTOR, IPL2SOFT) Timer3Handler(void)
{
   doLED_PWM();
   mT3ClearIntFlag(); // clear the interrupt flag
}

unsigned char G_red_cnt=0;
unsigned char G_red_pwm=0;

unsigned char G_green_cnt=0;
unsigned char G_green_pwm=0;

unsigned char G_blue_cnt=0;
unsigned char G_blue_pwm=0;

unsigned char G_flare_red_cnt=0;
unsigned char G_flare_red_pwm=0;

unsigned char G_flare_green_cnt=0;
unsigned char G_flare_green_pwm=0;

unsigned char G_flare_blue_cnt=0;
unsigned char G_flare_blue_pwm=0;

unsigned char G_bright=0;

unsigned char G_backlight=255;
unsigned char G_backlight_cnt=0;

unsigned char G_no_LED_PWM = 0;


void no_LED_PWM(unsigned char trueFalse) {
   G_no_LED_PWM = trueFalse;
}

void led_brightness(unsigned char bright) {
   G_bright = bright;
}

void red(unsigned char onPWM) {
    onPWM >>= G_bright;
    G_red_pwm = onPWM; 
    G_red_cnt = 0;
}

void green(unsigned char onPWM) {
    onPWM >>= G_bright;
    G_green_pwm = onPWM; 
    G_green_cnt = 0;
}

void blue(unsigned char onPWM) {
    onPWM >>= G_bright;
    G_blue_pwm = onPWM; 
    G_blue_cnt = 0;
}

void doLED_PWM()
{
    if (G_no_LED_PWM) return;

    G_backlight_cnt++;
    if (G_backlight_cnt < G_backlight)
        LATCbits.LATC9 = 1;
    else
        LATCbits.LATC9 = 0;

    /* red */
    G_red_cnt++;
    if (G_red_cnt < G_red_pwm)
        LATCbits.LATC0 = 1;
    else
        LATCbits.LATC0 = 0;

    // just let it wrap around if (G_red_cnt == 255) G_red_cnt = 0;

    /* Green */
    G_green_cnt++;
    if (G_green_cnt < G_green_pwm)
        LATBbits.LATB3 = 1;
    else
        LATBbits.LATB3 = 0;

    // just let it wrap around if (G_green_cnt == 255) G_green_cnt = 0;

    /* Blue */
    G_blue_cnt++;
    if (G_blue_cnt < G_blue_pwm)
        LATCbits.LATC1 = 1;
    else
        LATCbits.LATC1 = 0;

//    flarePWM();
}

void backlight(unsigned char bright) {
    G_backlight = bright; 
    G_backlight_cnt = 0;
}

void led(unsigned char r, unsigned char g, unsigned char b)
{
    red(r);
    green(g);
    blue(b);
}

void flareled(unsigned char r_pwm, unsigned char g_pwm, unsigned char b_pwm)
{
    G_flare_red_pwm = r_pwm;
    G_flare_green_pwm = g_pwm;
    G_flare_blue_pwm = b_pwm;

    G_flare_red_cnt = 0;
    G_flare_green_cnt = 0;
    G_flare_blue_cnt = 0;
}

void flarePWM()
{
    static int onled=0;

#ifdef PAULSHACKEDBADGE
    /*
	only one led can be on at a time
    */
    switch (onled) {
	case 0:
	    LATCbits.LATC4 = 0;
	    LATAbits.LATA4 = 0;

	    G_flare_red_cnt++;
	    if (G_flare_red_cnt < G_flare_red_pwm)
		LATCbits.LATC5 = 1;
	    else
		LATCbits.LATC5 = 0;
	    break;

	case 1:
	    LATCbits.LATC5 = 0;
	    LATAbits.LATA4 = 0;

	    G_flare_green_cnt++;
	    if (G_flare_green_cnt < G_flare_green_pwm)
		LATCbits.LATC4 = 1;
	    else
		LATCbits.LATC4 = 0;
	    break;

	case 2:
	    LATCbits.LATC4 = 0;
	    LATCbits.LATC5 = 0;

	    G_flare_blue_cnt++;
	    if (G_flare_blue_cnt < G_flare_blue_pwm)
		LATAbits.LATA4 = 1;
	    else
		LATAbits.LATA4 = 0;
	    break;
    }
    onled++;
    onled %= 3;
#endif
}
