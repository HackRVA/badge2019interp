#include "menu.h"
#include "ir.h"
#include "conductor.h"
#include "buttons.h"

struct menu_t conductor_config_m[] = {
    {"Top: ", VERT_ITEM, FUNCTION, {(struct menu_t *)set_conductor_top_note} },
    {"Bottom: ", VERT_ITEM, FUNCTION, {(struct menu_t *)set_conductor_bottom_note} },
    {"", VERT_ITEM|SKIP_ITEM, TEXT, 0},
    {"Left: ", VERT_ITEM, FUNCTION, {(struct menu_t *)set_conductor_left_note} },
    {"Right: ", VERT_ITEM, FUNCTION, {(struct menu_t *)set_conductor_right_note} },
    {"", VERT_ITEM|SKIP_ITEM, TEXT, 0},
    {"Mode: ", VERT_ITEM, FUNCTION, {(struct menu_t *)set_mode} },
    {"Go!!", VERT_ITEM|DEFAULT_ITEM, FUNCTION, {(struct menu_t *)set_go} },
    {"Exit", VERT_ITEM|LAST_ITEM, FUNCTION, {(struct menu_t *) set_exit}},
};


unsigned short top_note = 8;
unsigned short bottom_note = 32;
unsigned short left_note = 128;
unsigned short right_note = 512;

enum
{
    INIT,
    SHOW_MENU,
    CONFIG_TOP,
    CONFIG_BOTTOM,
    CONFIG_LEFT,
    CONFIG_RIGHT,
    RUN_CONDUCTOR
};

enum
{
    LOCAL_ONLY,
    BCAST_ONLY,
    LOCAL_AND_BCAST
};

char con_state = INIT;
char con_mode = LOCAL_ONLY;

void set_conductor_top_note()
{
    con_state = CONFIG_TOP;
}

void set_conductor_bottom_note()
{
    con_state = CONFIG_BOTTOM;
}

void set_conductor_left_note()
{
    con_state = CONFIG_LEFT;
}

void set_conductor_right_note()
{
    con_state = CONFIG_RIGHT;
}

void populate_menu();

void set_mode()
{
    if(con_mode == LOCAL_ONLY || con_mode == BCAST_ONLY)
        con_mode++;
    else
        con_mode = LOCAL_ONLY;
    
    populate_menu();
}

void set_go()
{
    con_state = RUN_CONDUCTOR;
}

void set_exit()
{
    con_state = INIT;
    returnToMenus();
}

void populate_menu()
{

    conductor_config_m[0].name[5] = '0' + (top_note/100) % 10;
    conductor_config_m[0].name[6] = '0' + (top_note/10) % 10;
    conductor_config_m[0].name[7] = '0' + top_note % 10;
    conductor_config_m[0].name[8] = 0;

    conductor_config_m[1].name[8] = '0' + (bottom_note/100) % 10;
    conductor_config_m[1].name[9] = '0' + (bottom_note/10) % 10;
    conductor_config_m[1].name[10] = '0' + bottom_note % 10;
    conductor_config_m[1].name[11] = 0;

    conductor_config_m[3].name[6] = '0' + (left_note/100) % 10;
    conductor_config_m[3].name[7] = '0' + (left_note/10) % 10;
    conductor_config_m[3].name[8] = '0' + left_note % 10;
    conductor_config_m[3].name[9] = 0;

    conductor_config_m[4].name[7] = '0' + (right_note/100) % 10;
    conductor_config_m[4].name[8] = '0' + (right_note/10) % 10;
    conductor_config_m[4].name[9] = '0' + right_note % 10;
    conductor_config_m[4].name[10] = 0;

    if(con_mode == LOCAL_ONLY)
    {
        conductor_config_m[6].name[6] = 'l';
        conductor_config_m[6].name[7] = 'o';
        conductor_config_m[6].name[8] = 'c';
        conductor_config_m[6].name[9] = 'a';
        conductor_config_m[6].name[10] = 'l';
        conductor_config_m[6].name[11] = 0;
    }
    else if(con_mode == BCAST_ONLY)
    {
        conductor_config_m[6].name[6] = 'b';
        conductor_config_m[6].name[7] = 'c';
        conductor_config_m[6].name[8] = 'a';
        conductor_config_m[6].name[9] = 's';
        conductor_config_m[6].name[10] = 't';
        conductor_config_m[6].name[11] = 0;
    }
    else if(con_mode == LOCAL_AND_BCAST)
    {
        conductor_config_m[6].name[6] = 'b';
        conductor_config_m[6].name[7] = 'o';
        conductor_config_m[6].name[8] = 't';
        conductor_config_m[6].name[9] = 'h';
        conductor_config_m[6].name[10] = 0;
    }
}


void run_conductor()
{
    union IRpacket_u pkt;
    unsigned short freq=0;

    if (BUTTON_PRESSED_AND_CONSUME)
    {
        //returnToMenus();
        con_state = SHOW_MENU;
    }

    if(UP_BTN)
    {
        freq = top_note;
    }

    if (DOWN_BTN)
    {
        freq = bottom_note;
    }

    if (LEFT_BTN)
    {
        freq = left_note;
    }

    if (RIGHT_BTN)
    {
        freq = right_note;
    }
    if(freq != 0)
    {
        if(con_mode == BCAST_ONLY || con_mode == LOCAL_AND_BCAST)
        {
            pkt.p.command = IR_WRITE;
            pkt.p.address = IR_LIVEAUDIO;
            pkt.p.badgeId = 0;
            pkt.p.data = (((2048 >> 8) & 0xF) << 12) | (freq & 0x0FFF) ;
            IRqueueSend(pkt);
        }

        if(con_mode == LOCAL_AND_BCAST || con_mode == LOCAL_ONLY)
            setNote(freq, 4096);
    }
}

void conductor_cb()
{
    switch(con_state)
    {
        case INIT:
            populate_menu();
            con_state++;
            break;
        case SHOW_MENU:
            genericMenu((struct menu_t *)conductor_config_m, MAIN_MENU_STYLE);
            break;
        case CONFIG_TOP:
            if (BUTTON_PRESSED_AND_CONSUME)
                con_state = SHOW_MENU;
                //returnToMenus();
            else if(UP_BTN_AND_CONSUME)
            {
                top_note++;
                setNote(top_note, 4096);
                populate_menu();
            }
            else if(DOWN_BTN_AND_CONSUME)
            {
                top_note--;
                setNote(top_note, 4096);
                populate_menu();
            }
            break;
        case CONFIG_BOTTOM:
            if (BUTTON_PRESSED_AND_CONSUME)
                con_state = SHOW_MENU;
                //returnToMenus();
            else if(UP_BTN_AND_CONSUME)
            {
                bottom_note++;
                setNote(bottom_note, 4096);
                populate_menu();
            }
            else if(DOWN_BTN_AND_CONSUME)
            {
                bottom_note--;
                setNote(bottom_note, 4096);
                populate_menu();
            }
            break;
        case CONFIG_LEFT:
            if (BUTTON_PRESSED_AND_CONSUME)
                con_state = SHOW_MENU;
                //returnToMenus();
            else if(UP_BTN_AND_CONSUME)
            {
                left_note++;
                setNote(left_note, 4096);
                populate_menu();
            }
            else if(DOWN_BTN_AND_CONSUME)
            {
                left_note--;
                setNote(left_note, 4096);
                populate_menu();
            }
            break;
        case CONFIG_RIGHT:
            if (BUTTON_PRESSED_AND_CONSUME)
                con_state = SHOW_MENU;
                //returnToMenus();
            else if(UP_BTN_AND_CONSUME)
            {
                right_note++;
                setNote(right_note, 4096);
                populate_menu();
            }
            else if(DOWN_BTN_AND_CONSUME)
            {
                right_note--;
                setNote(right_note, 4096);
                populate_menu();
            }
            break;
        case RUN_CONDUCTOR:
            run_conductor();
            break;
    }

}
