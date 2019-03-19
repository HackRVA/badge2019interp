#include <string.h>
#include "colors.h"
#include "fb.h"
#include "buttons.h"
#include "adc.h"
#include "timer1_int.h"

extern char hextab[];

// exercise the ADC
// peb 20180228
// todo: a bigger buffer with downscaling

enum adc_state {
    INIT,
    INPUT,
    DRAW,
    EXIT
};

void adc_cb() {
   static int cnt=0;
   static unsigned char state = INIT;
   int i, ADCloopCnt=0, changed=0;
   char title[32];
   static int hz_num=0;
   static int analog_src_num=0;

   // pwm creates ADC noise
   no_LED_PWM(1);

   switch(state){
	case INIT:
	    FbBackgroundColor(BLACK);
	    FbClear();

	    FbColor(WHITE);
	    FbMove(10,10);

	    strcpy(title, "kHZ ");
	    strcat(title, samples_info[hz_num].name);
	    FbWriteLine(title);

	    FbMove(10,20);
	    strcpy(title, "left/right: HZ ");
	    FbWriteLine(title);

	    FbMove(10,30);
	    strcpy(title, "up/down: mode ");
	    FbWriteLine(title);

	    FbMove(10,40);
	    strcpy(title, analog_info[analog_src_num].name);
	    FbWriteLine(title);

	    FbMove(10,50);
	    strcpy(title, "Press start");
	    FbWriteLine(title);

	    FbSwapBuffers();

	    state++;
	    break;

	case INPUT:
	    if (BUTTON_PRESSED_AND_CONSUME){ // delay to read
		ADC_init(analog_src_num, hz_num); // init w/ current hz_num
		state++;
		cnt = 0;
	    }
	    if (UP_BTN_AND_CONSUME) { // changes mask
		analog_src_num--;
		if (analog_src_num<0) analog_src_num=0; 
		state = INIT;
		cnt = 0;
	    }

	    if (DOWN_BTN_AND_CONSUME) {
		analog_src_num++;
		if (analog_src_num == AN_LAST) analog_src_num--;
		state = INIT;
		cnt = 0;
	    }

	    // allow hz change here
	    if (LEFT_BTN_AND_CONSUME) {
		hz_num--;
		if (hz_num < 0) hz_num = 0;
		state = INIT;
		cnt = 0;
	    }

	    if (RIGHT_BTN_AND_CONSUME) {
		hz_num++;
		if (hz_num == HZ_LAST) hz_num--;
		state = INIT;
		cnt = 0;
	    }

	    break;

	case DRAW:
	    if(BUTTON_PRESSED_AND_CONSUME) state++;

	    // allow hz change here also
	    if (LEFT_BTN_AND_CONSUME) {
		hz_num--;
		if (hz_num < 0) hz_num = 0;
		state = INIT; // reinit app
		cnt = 0;
	    }

	    if (RIGHT_BTN_AND_CONSUME) {
		hz_num++;
		if (hz_num == HZ_LAST) hz_num--;
		state = INIT; // reinit app
		cnt = 0;
	    }

	    if (UP_BTN_AND_CONSUME) { // changes mask
		analog_src_num--;
		if (analog_src_num<0) analog_src_num=0; 
		state = INIT; // reinit app
		cnt = 0;
	    }

	    if (DOWN_BTN_AND_CONSUME) {
		analog_src_num++;
		if (analog_src_num == AN_LAST) analog_src_num--;
		state = INIT; // reinit app
		cnt = 0;
	    }

	    if (ADCbufferCntMark == 0) { // has started filling buffer yet
		if (ADCbufferCnt >= ADC_BUFFER_SIZE ) { // ADC has filled
		    unsigned short min[4], max[4], delta[4], Lshift[4], Rshift[4];
		    unsigned char b, s, x, y;

		    // init min and maxes- #chans=4
		    for (s=0; s<G_chans; s++) {
			   min[s] = 0xFFFF;
			   max[s] = 0x0000;
			   Rshift[s] = 0;
			   Lshift[s] = 0;
		    }

		    // find min/max for channels
		    for (i=0; i < ADC_BUFFER_SIZE; i+=G_chans) {
			   for (s=0; s<G_chans; s++) {
				if (ADCbuffer[i+s] < min[s]) min[s] = ADCbuffer[i+s];
				if (ADCbuffer[i+s] > max[s]) max[s] = ADCbuffer[i+s];
			    }
		    }

		    for (s=0; s<G_chans; s++) {
			   delta[s] = max[s] - min[s];
		    }

		    // find highest bit to for scale down
		    for (b=9; b>=5; b--) {
			   for (s=0; s<G_chans; s++) {
			      if (delta[s] & (1<<b))    { if (Rshift[s]==0)    Rshift[s] = b-4; };
			   }
		    }
		    // find highest bit for scale up
		    // if the levels are really low (b<=1) this just amplifies noise
		    // the dev version has better filtering on AVss 
		    for (b=4; b>2; b--) {
			   for (s=0; s<G_chans; s++) {
			      if (delta[s] & (1<<b)) { 
				if ((Rshift[s]==0) & (Lshift[s] == 0)) Lshift[s] = 4-b;
			      }
			   }
		    }

		    // the ADC samples and buffers each pin in sequence, 
		    // need to pic them apart and plot them on their own line
		    // avoid division when posible
		    for (i=0,x=0; i < ADC_BUFFER_SIZE; i+=G_chans,x++) { // RF 4=128 buf, 8 = 256 buffer
			   for (s=0; s<G_chans; s++) {
				if (s==0) {
				   y = 8;
				   FbColor(B_RED);
				}

				if (s==1) {
				   y = 40;
				   FbColor(GREEN);
				}

				if (s==2) {
				   y = 72;
				   FbColor(B_BLUE);
				}
				if (s==3) {
				   y = 96;
				   FbColor(WHITE);
				}
				FbPoint(x,   y +   (((ADCbuffer[i+s] - min[s]) << Lshift[s]) >> Rshift[s]));
			   }
		    }

		    // handshake to empty buffer and start aquiring again
		    ADCbufferCntMark = 1; 

//		    ADCloopCnt++;
//		    // need to make this controlable from the UI
//		    if (ADCloopCnt==3) { // clear FB after a bit
//			ADCloopCnt=0;
//			FbSwapBuffers();
//		    }
//		    else
//			FbPushBuffer();

		    FbSwapBuffers();
		}
	    }
	    break;

	case EXIT:
	    //FbBackgroundColor(GREY1);
	    state = INIT;
	    cnt = 0;
	    no_LED_PWM(0);
	        
	    ADC_stop(); 
	        
	    returnToMenus();
	    break;
    } // end switch
    cnt++;
}
