/* 
 * File:   buttons.h
 * Author: morgan
 *
 * Created on April 1, 2017, 9:11 PM
 */

#ifndef BUTTONS_H
#define	BUTTONS_H

//-----------------------
//----Tactile Buttons----
extern unsigned char G_button_cnt; // Standalone in bottom right

//D Pad
extern unsigned char G_up_button_cnt;
extern unsigned char G_down_button_cnt;
extern unsigned char G_left_button_cnt;
extern unsigned char G_right_button_cnt;

// Mask of buttons that have had their current press used
extern unsigned char G_pressed_button;
// Mix some randomness up
extern unsigned int G_entropy_pool;

extern unsigned int timestamp;
extern unsigned int last_input_timestamp;
#define SOLO_BTN_MASK  1
#define UP_BTN_MASK    2
#define DOWN_BTN_MASK  4
#define LEFT_BTN_MASK  8
#define RIGHT_BTN_MASK 16
#define ALL_BTN_MASK   0b00011111

#define UP_TOUCH_MASK      32
#define MIDDLE_TOUCH_MASK  64
#define DOWN_TOUCH_MASK    128
#define ALL_TOUCH_MASK     0b11100000

#define IN_MASK(V, MASK) (V & MASK)
#define ADD_TO_MASK(V, MASK) (V |= MASK)
#define REMOVE_FROM_MASK(V, MASK) (V &= (~MASK))
#define CHECK_THEN_ADD_TO_MASK(V, MASK) (!IN_MASK(V, MASK) && ADD_TO_MASK(V, MASK))

#define DEFAULT_BTN_DBC 1
// '*_CONSUME' macros return True once for each single
// button press, no matter how long the button is held
#define BUTTON_PRESSED_AND_CONSUME (G_button_cnt > DEFAULT_BTN_DBC && CHECK_THEN_ADD_TO_MASK(G_pressed_button, SOLO_BTN_MASK))

#define DOWN_BTN_AND_CONSUME (G_down_button_cnt > DEFAULT_BTN_DBC && CHECK_THEN_ADD_TO_MASK(G_pressed_button, DOWN_BTN_MASK))
#define UP_BTN_AND_CONSUME (G_up_button_cnt > DEFAULT_BTN_DBC && CHECK_THEN_ADD_TO_MASK(G_pressed_button, UP_BTN_MASK))
#define LEFT_BTN_AND_CONSUME (G_left_button_cnt > DEFAULT_BTN_DBC && CHECK_THEN_ADD_TO_MASK(G_pressed_button, LEFT_BTN_MASK))
#define RIGHT_BTN_AND_CONSUME (G_right_button_cnt > DEFAULT_BTN_DBC && CHECK_THEN_ADD_TO_MASK(G_pressed_button, RIGHT_BTN_MASK))

// More simple methods to test if the button is currently pressed for 
// a certain amount of counts (debounce)
#define DOWN_BTN_HOLD(HOLD_TIME) (G_down_button_cnt > HOLD_TIME)
#define UP_BTN_HOLD(HOLD_TIME)  (G_up_button_cnt > HOLD_TIME)
#define LEFT_BTN_HOLD(HOLD_TIME) (G_left_button_cnt > HOLD_TIME)
#define RIGHT_BTN_HOLD(HOLD_TIME) (G_right_button_cnt > HOLD_TIME)

#define DOWN_BTN DOWN_BTN_HOLD(DEFAULT_BTN_DBC)
#define UP_BTN UP_BTN_HOLD(DEFAULT_BTN_DBC)
#define LEFT_BTN LEFT_BTN_HOLD(DEFAULT_BTN_DBC)
#define RIGHT_BTN RIGHT_BTN_HOLD(DEFAULT_BTN_DBC)


//--------------------
//-----Cap Touch------
extern unsigned char G_up_touch_cnt;
extern unsigned char G_middle_touch_cnt;
extern unsigned char G_down_touch_cnt;
extern char G_touch_pct;

// PEB #define DEFAULT_TOUCH_DBC 4
#define DEFAULT_TOUCH_DBC 20
#define DOWN_TOUCH_AND_CONSUME (G_down_touch_cnt > DEFAULT_TOUCH_DBC  && CHECK_THEN_ADD_TO_MASK(G_pressed_button, DOWN_TOUCH_MASK))
#define MIDDLE_TOUCH_AND_CONSUME (G_middle_touch_cnt > DEFAULT_TOUCH_DBC  && CHECK_THEN_ADD_TO_MASK(G_pressed_button, MIDDLE_TOUCH_MASK))
#define UP_TOUCH_AND_CONSUME (G_up_touch_cnt > DEFAULT_TOUCH_DBC  && CHECK_THEN_ADD_TO_MASK(G_pressed_button, UP_TOUCH_MASK))

#define DOWN_TOUCH_HOLD(HOLD_TIME) (G_down_touch_cnt > HOLD_TIME)
#define MIDDLE_TOUCH_HOLD(HOLD_TIME) (G_middle_touch_cnt > HOLD_TIME)
#define UP_TOUCH_HOLD(HOLD_TIME)  (G_up_touch_cnt > HOLD_TIME)

#define DOWN_TOUCH DOWN_TOUCH_HOLD(DEFAULT_TOUCH_DBC)
#define MIDDLE_TOUCH MIDDLE_TOUCH_HOLD(DEFAULT_TOUCH_DBC)
#define UP_TOUCH UP_TOUCH_HOLD(DEFAULT_TOUCH_DBC)

#define TIME_SINCE_LAST_INPUT (timestamp - last_input_timestamp)

void clear_buttons();

void button_task(void *p_arg);

int getButton();
int getDPAD();

#endif	/* BUTTONS_H */

