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

    // Standalone/main button
    if(!(PORTBbits.RB14 != 0)){
	if(G_button_cnt < 255)
	    G_button_cnt++;
    }
    else{
	G_button_cnt = 0;
	REMOVE_FROM_MASK(G_pressed_button, SOLO_BTN_MASK);
    }
    
    //UP:
    if(!(PORTBbits.RB2 != 0)){
	if(G_up_button_cnt < 255)
	    G_up_button_cnt++;
    }
    else{
	
	G_up_button_cnt = 0;
	REMOVE_FROM_MASK(G_pressed_button, UP_BTN_MASK);
    }
    
    //LEFT:
    if(!(PORTBbits.RB0 != 0)){
	if(G_left_button_cnt < 255)
	    G_left_button_cnt++;
    }
    else{
	G_left_button_cnt = 0;
	REMOVE_FROM_MASK(G_pressed_button, LEFT_BTN_MASK);
    }
    
    //RIGHT:
    if(!(PORTCbits.RC2 != 0)){
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
