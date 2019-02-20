#include <plib.h>
#include "buttons.h"

unsigned char G_pressed_button = 0;

// Solo bottom right btn
unsigned char G_button_cnt = 0;

// D-Pad
unsigned char G_up_button_cnt = 0;
unsigned char G_down_button_cnt = 0;
unsigned char G_left_button_cnt = 0;
unsigned char G_right_button_cnt = 0;

unsigned int timestamp = 0;
unsigned int last_input_timestamp = 0;

void doButtons()
{
    //---------------------
    //---Tactile Buttons---
    // If btn is being pressed
    //   - increment its counter, but don't overflow
    // else if btn is not being pressed
    //   - Zero its counter and zero out 'pressed' bit flag


    /*
	RB14 == main button

	2019 D-PAD

	RC2 == RIGHT
	RB2 == TOP/UP
	RB1 == BOTTOM/DOWN
	RB0 == LEFT
    */

    timestamp++;

    /*
	buttons are pulled high, 
	so zero is pressed
    */
    // Standalone/main button
    if(PORTBbits.RB14 == 0){
	if(G_button_cnt < 255)
	    G_button_cnt++;
    }
    else{
	G_button_cnt = 0;
	REMOVE_FROM_MASK(G_pressed_button, SOLO_BTN_MASK);
    }
    
    //UP:
    if(PORTBbits.RB2 == 0){
	if(G_up_button_cnt < 255)
	    G_up_button_cnt++;
    }
    else{
	G_up_button_cnt = 0;
	REMOVE_FROM_MASK(G_pressed_button, UP_BTN_MASK);
    }
    
    //LEFT:
    if(PORTBbits.RB0 == 0){
	if(G_left_button_cnt < 255)
	    G_left_button_cnt++;
    }
    else{
	G_left_button_cnt = 0;
	REMOVE_FROM_MASK(G_pressed_button, LEFT_BTN_MASK);
    }
    
    //RIGHT:
    if(PORTCbits.RC2 == 0){
	if(G_right_button_cnt < 255)
	    G_right_button_cnt++;
    }
    else{
	G_right_button_cnt = 0;
	REMOVE_FROM_MASK(G_pressed_button, RIGHT_BTN_MASK);
    }
    
    //DOWN:
    if(!(PORTBbits.RB1 != 0)){
	G_down_button_cnt++;
    }
    else{
	G_down_button_cnt = 0;
	REMOVE_FROM_MASK(G_pressed_button, DOWN_BTN_MASK);
    }
    
    if(DOWN_BTN || UP_BTN || LEFT_BTN || RIGHT_BTN || (G_button_cnt > DEFAULT_BTN_DBC)){
	last_input_timestamp = timestamp;
    }
}


void clear_buttons(){
    G_button_cnt = 0;
    G_up_button_cnt = 0;
    G_down_button_cnt = 0;
    G_left_button_cnt = 0;
    G_right_button_cnt = 0;
}

/*
   these read directly for the interpreter
   because it is outside the main event 
   loop which is consuming the 
   buttons presses
*/
int getButton()
{
    int b=0;
    static int lastb=0;
    static unsigned int now=0, last=0;

    now = _CP0_GET_COUNT();
    if (now < last) last = now; // wrapped

    b |= (PORTBbits.RB14 == 0) ? SOLO_BTN_MASK : 0;

    if (now > (last+4000000)) {
	lastb = b;
        last = _CP0_GET_COUNT(); // this is in microseconds 1/1000000 and wraps after 2^32/40mhz ~107 seconds
	b = 0;
    }
    else 
	lastb = 0;

    return (lastb);
}

int getDPAD()
{
    int b=0;
    static int lastb=0;
    static unsigned int now=0, last=0;

    now = _CP0_GET_COUNT();
    if (now < last) last = now; // wrapped

    b |= (PORTBbits.RB2 == 0) ? UP_BTN_MASK : 0;
    b |= (PORTBbits.RB0 == 0) ? LEFT_BTN_MASK : 0;
    b |= (PORTCbits.RC2 == 0) ? RIGHT_BTN_MASK : 0;
    b |= (PORTBbits.RB1 == 0) ? DOWN_BTN_MASK : 0;

    if (now > (last+4000000)) { // 1/10 sec  4,000,000 / 40,000,000
	lastb = b;
        last = _CP0_GET_COUNT();
	b=0;
    }
    else 
	lastb = 0;

    return (lastb);
}
