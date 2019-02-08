//#include "badge16.h"
#include "colors.h"
#include "badge_menu.h"
#include "ir.h"
//#include "touchCTMU.h"
#include "buttons.h"
//#include "blinkenlights.h"

//DECLARE
#define BL_INCR_AMNT 2
void set_red();
void set_blue();
void set_green();

void bl_clear_colors();

void set_bl_mode();
void set_bl_go();
void set_bl_exit();

void set_local_leds();
void bl_populate_menu();

void blinkenlights_task(void *args);


// If ordering changed, make sure indices still work
// in the populate function
struct menu_t blinkenlights_config_m[] = {
    {"Red: ", VERT_ITEM, FUNCTION, {(struct menu_t *)set_red}},
    {"Blue: ", VERT_ITEM, FUNCTION, {(struct menu_t *)set_blue}},
    {"Green: ", VERT_ITEM, FUNCTION, {(struct menu_t *)set_green}},
    {"--CLEAR--", VERT_ITEM, FUNCTION, {(struct menu_t *)bl_clear_colors}},
    {"", VERT_ITEM|SKIP_ITEM, TEXT, 0},
    {"Mode: ", VERT_ITEM, FUNCTION, {(struct menu_t *)set_bl_mode}},
    {"Go!!", VERT_ITEM|DEFAULT_ITEM, FUNCTION, {(struct menu_t *)set_bl_go}},
    {"Exit", VERT_ITEM|LAST_ITEM, FUNCTION, {(struct menu_t *) set_bl_exit}},
};
unsigned char bl_red = 50, bl_green = 40, bl_blue = 0;

char image = 0;

enum
{
    INIT,
    SHOW_MENU,
    CONFIG_RED,
    CONFIG_BLUE,
    CONFIG_GREEN,
    RUN_BLINKENLIGHTS
};

enum
{
    LOCAL_ONLY,
    BCAST_ONLY,
    LOCAL_AND_BCAST
};

char bl_state = INIT;
char bl_mode = LOCAL_ONLY;

void set_red()
{
    bl_state = CONFIG_RED;
}

void set_blue()
{
    bl_state = CONFIG_BLUE;
}

void set_green()
{
    bl_state = CONFIG_GREEN;
}

void bl_clear_colors()
{
    bl_red = 0;
    bl_green = 0;
    bl_blue = 0;
    bl_populate_menu();
}

void set_bl_mode()
{
    if(bl_mode == LOCAL_ONLY || bl_mode == BCAST_ONLY)
        bl_mode++;
    else
        bl_mode = LOCAL_ONLY;

    bl_populate_menu();
}

void set_bl_go()
{
    union IRpacket_u pkt;
    if(bl_mode == BCAST_ONLY || bl_mode == LOCAL_AND_BCAST)
    {
        pkt.p.command = IR_WRITE;
        pkt.p.address = IR_LED;
        pkt.p.badgeId = 0;
        pkt.p.data = PACKRGB( bl_red, bl_green, bl_blue);
        IRqueueSend(pkt);
    }

    if(bl_mode == LOCAL_ONLY || bl_mode == LOCAL_AND_BCAST)
    {
        set_local_leds();
    }
}

void set_bl_exit()
{
    bl_state = INIT;
    returnToMenus();
}

void bl_populate_menu()
{
    blinkenlights_config_m[0].name[5] = '0' + (bl_red/100) % 10;
    blinkenlights_config_m[0].name[6] = '0' + (bl_red/10) % 10;
    blinkenlights_config_m[0].name[7] = '0' + bl_red % 10;
    blinkenlights_config_m[0].name[8] = 0;

    blinkenlights_config_m[1].name[6] = '0' + (bl_blue/100) % 10;
    blinkenlights_config_m[1].name[7] = '0' + (bl_blue/10) % 10;
    blinkenlights_config_m[1].name[8] = '0' + bl_blue % 10;
    blinkenlights_config_m[1].name[9] = 0;

    blinkenlights_config_m[2].name[7] = '0' + (bl_green/100) % 10;
    blinkenlights_config_m[2].name[8] = '0' + (bl_green/10) % 10;
    blinkenlights_config_m[2].name[9] = '0' + bl_green % 10;
    blinkenlights_config_m[2].name[10] = 0;
    
    if(bl_mode == LOCAL_ONLY)
    {
        blinkenlights_config_m[5].name[6] = 'l';
        blinkenlights_config_m[5].name[7] = 'o';
        blinkenlights_config_m[5].name[8] = 'c';
        blinkenlights_config_m[5].name[9] = 'a';
        blinkenlights_config_m[5].name[10] = 'l';
        blinkenlights_config_m[5].name[11] = 0;
    }
    else if(bl_mode == BCAST_ONLY)
    {
        blinkenlights_config_m[5].name[6] = 'b';
        blinkenlights_config_m[5].name[7] = 'c';
        blinkenlights_config_m[5].name[8] = 'a';
        blinkenlights_config_m[5].name[9] = 's';
        blinkenlights_config_m[5].name[10] = 't';
        blinkenlights_config_m[5].name[11] = 0;
    }
    else if(bl_mode == LOCAL_AND_BCAST)
    {
        blinkenlights_config_m[5].name[6] = 'b';
        blinkenlights_config_m[5].name[7] = 'o';
        blinkenlights_config_m[5].name[8] = 't';
        blinkenlights_config_m[5].name[9] = 'h';
        blinkenlights_config_m[5].name[10] = 0;
    }
}

void set_local_leds()
{
    led(bl_red,bl_green,bl_blue);
}

void blinkenlights_task(void *args)
{
    for(;;){
        switch(bl_state)
        {
            case INIT:
                bl_populate_menu();
                bl_state++;
                break;
            case SHOW_MENU:
                genericMenu((struct menu_t *)blinkenlights_config_m, MAIN_MENU_STYLE);
                FbSwapBuffers();
                break;
            case CONFIG_RED:
                if(BUTTON_PRESSED_AND_CONSUME)
                {
                    led(0,0,0);
                    bl_state = SHOW_MENU;
                }
                else if(UP_BTN_AND_CONSUME)
                {
                    bl_red += BL_INCR_AMNT;
                    if(bl_red > 100)
                        bl_red = 100;

                    set_local_leds();
                    bl_populate_menu();

                }
                else if(DOWN_BTN_AND_CONSUME)
                {
                    if(bl_red  > BL_INCR_AMNT)
                        bl_red -= BL_INCR_AMNT;
                    else
                        bl_red = 0;

                    set_local_leds();
                    bl_populate_menu();
                }
                break;
            case CONFIG_GREEN:
                if(BUTTON_PRESSED_AND_CONSUME)
                {
                    red(0);
                    green(0);
                    blue(0);
                    bl_state = SHOW_MENU;
                }
                else if(UP_BTN_AND_CONSUME)
                {
                    bl_green += BL_INCR_AMNT;
                    if(bl_green > 100)
                        bl_green = 100;

                    set_local_leds();
                    bl_populate_menu();

                }
                else if(DOWN_BTN_AND_CONSUME)
                {
                    if(bl_green > BL_INCR_AMNT)
                        bl_green -= BL_INCR_AMNT;
                    else
                        bl_green = 0;

                    set_local_leds();
                    bl_populate_menu();
                }
                break;
            case CONFIG_BLUE:
                if(BUTTON_PRESSED_AND_CONSUME)
                {
                    red(0);
                    green(0);
                    blue(0);
                    bl_state = SHOW_MENU;
                }
                else if(UP_BTN_AND_CONSUME)
                {
                    bl_blue += BL_INCR_AMNT;
                    if(bl_blue > 100)
                        bl_blue = 100;

                    set_local_leds();
                    bl_populate_menu();

                }
                else if(DOWN_BTN_AND_CONSUME)
                {
                    if(bl_blue  > BL_INCR_AMNT)
                        bl_blue -= BL_INCR_AMNT;
                    else
                        bl_blue = 0;

                    set_local_leds();
                    bl_populate_menu();
                }            
                break;
        }
    }
}
