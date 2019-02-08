// badge
#include "colors.h"
#define NULL 0
#ifdef SDL_BADGE
#include "sdl_fb.h"
#include "sdl_buttons.h"
#else
//#include "queue.h"
#include "fb.h"
#include "buttons.h"
#include "settings.h"
#include "timer1_int.h"
#include "flash.h"
#endif
#include "badge_menu.h"
#include "colors.h"
#include "assetList.h"

struct menuStack_t {
   struct menu_t *selectedMenu;
   struct menu_t *currMenu;
};

#define MAX_MENU_DEPTH 10
static unsigned char G_menuCnt=0; // index for G_menuStack

struct menuStack_t G_menuStack[MAX_MENU_DEPTH] = { {0,0} }; // track user traversing menus
struct menu_t *G_selectedMenu = NULL; /* item the cursor is on */
struct menu_t *G_currMenu = NULL; /* init */

#define MAIN_MENU_BKG_COLOR GREY2
#define MAIN_MENU_SCRATCH_X 3
#define MAIN_MENU_SCRATCH_Y 52
#define MAIN_MENU_SCRATCH_WIDTH 120
#define MAIN_MENU_SCRATCH_HEIGHT 73

unsigned char menu_left = 5;
//const unsigned char menu_msg_alert[] = "New Message!";
/* these should all be variables or part of a theme */
#define MENU_LEFT menu_left
#define CHAR_WIDTH 9//assetList[G_Fb.font].x
#define CHAR_HEIGHT 8
#define SCAN_BLANK 1 /* blank lines between text entries */
#define TEXTRECT_OFFSET 1 /* text offset within rectangle */

#define RGBPACKED(R,G,B) ( ((unsigned short)(R)<<11) | ((unsigned short)(G)<<6) | (unsigned short)(B) )


struct menu_t *getSelectedMenu() {
    return G_selectedMenu;
}

struct menu_t *getCurrMenu() {
    return G_currMenu;
}

struct menu_t *getMenuStack(unsigned char item) {
    if (item > G_menuCnt) return 0;

    return G_menuStack[G_menuCnt - item].currMenu;
}

struct menu_t *getSelectedMenuStack(unsigned char item) {
    if (item > G_menuCnt) return 0;

    return G_menuStack[G_menuCnt - item].selectedMenu;
}

#define PAGESIZE 8
void genericMenu(struct menu_t *L_menu, MENU_STYLE style) {
    static struct menu_t *L_currMenu = NULL; /* LOCAL not to be confused to much with menu()*/
    static struct menu_t *L_selectedMenu = NULL; /* LOCAL ditto   "    "    */
    static unsigned char L_menuCnt = 0; // index for G_menuStack
    static struct menu_t * L_menuStack[4] = {0}; // track user traversing menus

    if (L_menu == NULL) return; /* no thanks */

    if (L_currMenu == NULL) {
        L_menuCnt = 0;
        L_menuStack[L_menuCnt] = L_menu;
        L_currMenu = L_menu;
        //L_selectedMenu = L_menu;
        L_selectedMenu = NULL;
        L_selectedMenu = display_menu(L_currMenu, L_selectedMenu, style);
        return;
    }

    if (BUTTON_PRESSED_AND_CONSUME) {
        switch (L_selectedMenu->type) {

            case MORE: /* jump to next page of menu */
                setNote(173, 2048); /* a */
                L_currMenu += PAGESIZE;
                L_selectedMenu = L_currMenu;
                break;

            case BACK: /* return from menu */
                setNote(154, 2048); /* b */
                if (L_menuCnt == 0) return; /* stack is empty, error or main menu */
                L_menuCnt--;
                L_currMenu = L_menuStack[L_menuCnt];
                L_selectedMenu = L_currMenu;
                L_selectedMenu = display_menu(L_currMenu, L_selectedMenu, style);
                break;

            case TEXT: /* maybe highlight if clicked?? */
                setNote(145, 2048); /* c */
                break;

            case MENU: /* drills down into menu if clicked */
                //setNote(129, 2048); /* d */
                L_menuStack[L_menuCnt++] = L_currMenu; /* push onto stack  */
                if (L_menuCnt == MAX_MENU_DEPTH) L_menuCnt--; /* too deep, undo */
                L_currMenu = (struct menu_t *) L_selectedMenu->data.menu; /* go into this menu */
                //L_selectedMenu = L_currMenu;
                L_selectedMenu = NULL;
                L_selectedMenu = display_menu(L_currMenu, L_selectedMenu, style);
                break;

            case FUNCTION: /* call the function pointer if clicked */
                //Hack alert: the functino call may not returned
                // now that we are using tasks, so zero some of these
                // out before the call
                L_currMenu = NULL;
                setNote(115, 2048); /* e */
                (*L_selectedMenu->data.func)(L_selectedMenu);

                /* clean up for nex call back */
                L_menu = NULL;
                //L_currMenu = NULL;
                L_selectedMenu = NULL;

                L_menuCnt = 0;
                L_menuStack[L_menuCnt] = NULL;
                break;

            default:
                break;
        }
        // L_selectedMenu = display_menu(L_currMenu, L_selectedMenu);
    } else if (UP_BTN_AND_CONSUME) /* handle slider/soft button clicks */ {
        setNote(109, 2048); /* f */

        /* make sure not on first menu item */
        if (L_selectedMenu > L_currMenu) {
            L_selectedMenu--;

            while ((L_selectedMenu->attrib & SKIP_ITEM)
                   && L_selectedMenu > L_currMenu)
                L_selectedMenu--;

            L_selectedMenu = display_menu(L_currMenu, L_selectedMenu, style);
        }
    } else if (DOWN_BTN_AND_CONSUME) {
        setNote(97, 2048); /* g */

        /* make sure not on last menu item */
        if (!(L_selectedMenu->attrib & LAST_ITEM)) {
            L_selectedMenu++;

            //Last item should never be a skipped item!!
            while (L_selectedMenu->attrib & SKIP_ITEM)
                L_selectedMenu++;

            L_selectedMenu = display_menu(L_currMenu, L_selectedMenu, style);
        }
    }
}

struct menu_t *display_menu(struct menu_t *menu,
                            struct menu_t *selected,
                            MENU_STYLE style)
{
    static unsigned char cursor_x, cursor_y;
    unsigned char c;
    struct menu_t *root_menu; /* keep a copy in case menu has a bad structure */

    root_menu = menu;

    switch (style)
    {
        //case MAIN_MENU_WITH_TIME_DATE_STYLE:
        case MAIN_MENU_STYLE:
            FbBackgroundColor(MAIN_MENU_BKG_COLOR);
            FbClear();
            
            FbColor(GREEN);
            FbMove(2,5);
            FbRectangle(123, 120);

            FbColor(CYAN);
            FbMove(1,4);
            FbRectangle(125, 122);

            break;
        case WHITE_ON_BLACK:
            FbClear();
            FbBackgroundColor(BLACK);
            FbTransparentIndex(0);
            break;
        case BLANK:
            break;
    }

    cursor_x = MENU_LEFT;
    cursor_y = 2; // CHAR_HEIGHT;
    FbMove(cursor_x, cursor_y);

    while (1) {
        unsigned char rect_w=0;

        if (menu->attrib & HIDDEN_ITEM) {
            // don't jump out of the menu array if this is the last item!
            if(menu->attrib & LAST_ITEM)
                break;
            else
                menu++;

            continue;
        }

        for (c=0, rect_w=0; (menu->name[c] != 0); c++) rect_w += CHAR_WIDTH;

        if (menu->attrib & VERT_ITEM)
                cursor_y += (CHAR_HEIGHT + 2 * SCAN_BLANK);

        if (!(menu->attrib & HORIZ_ITEM))
                cursor_x = MENU_LEFT;

        // extra decorations for menu items
        switch(style)
        {
            case MAIN_MENU_STYLE:
                break;
            case WHITE_ON_BLACK:
                //FbMove(cursor_x, cursor_y);
                //FbFilledRectangle(rect_w, CHAR_HEIGHT + 2 * SCAN_BLANK);
                break;
            case BLANK:
                break;
        }
        if (selected == menu) {
            // If we happen to be on a skip ITEM, just increment off it
            // The menus() method mostly avoids this, except for some cases
            if (menu->attrib & SKIP_ITEM) selected++;
        }

        if (selected == NULL) {
            if (menu->attrib & DEFAULT_ITEM)
            selected = menu;
        }

        // Determine selected item color
        switch(style)
        {
            case MAIN_MENU_STYLE:
                if (menu == selected)
                {
                    FbColor(YELLOW);

                    FbMove(3, cursor_y+1);
                    FbFilledRectangle(2,8);

                    // Set the selected color for the coming writeline
                    FbColor(GREEN);
                }
                else
                    // unselected writeline color
                    FbColor(GREY16);
                break;
            case WHITE_ON_BLACK:
                FbColor((menu == selected) ? GREEN : WHITE);
                break;
            case BLANK:
                break;
        }


        FbMove(cursor_x+1, cursor_y+1);
        FbWriteLine(menu->name);
        cursor_x += (rect_w + CHAR_WIDTH);
        if (menu->attrib & LAST_ITEM) break;
            menu++;
    }// END WHILE

//        if (!(menu->attrib & HORIZ_ITEM))
//            cursor_x = MENU_LEFT;
//
//    #define FUDGE 4
//        FbColor(WHITE);
//        FbMove(MENU_LEFT, 128 - (CHAR_HEIGHT + 2 * SCAN_BLANK) - FUDGE);
//        FbWriteLine(out);
//    }
    // Display the badge ID in the upper right hand corner of the main menu
    // - This condition doesn't pass on boot, which of these isn't True
//    if((G_currMenu == main_m) && (runningApp == NULL || runningApp == splash_cb)){
//        unsigned char bid[4] = {'0' + G_sysData.badgeId/100 % 10,
//                                '0' + G_sysData.badgeId/10 % 10,
//                                '0' + G_sysData.badgeId % 10,
//                                0};
//        FbColor(WHITE);
//        FbMove(102,7);
//        FbWriteLine(bid);
//
//        FbColor(GREEN);
//        FbMove(100,5);
//        FbRectangle(25, 11);
//    }

    // Display a notification to the user if they have a message
    // But only if they are in the main menu
//    if(new_message && G_menuStack[G_menuCnt].currMenu == main_m && runningApp == NULL){
//        FbMove(4, 69);
//        FbColor(WHITE);
//        FbFilledRectangle(95, 10);
//
//        FbMove(5, 70);
//        FbColor(BLACK);
//        FbWriteWrappedLine(menu_msg_alert);
//
//        FbMove(2, 79);
//        FbColor(WHITE);
//        FbRectangle(123, 46);
//
//        FbMove(5, 81);
//        FbColor(YELLOW);
//        unsigned char c_str[MAX_MSG_LENGTH];
//        // lol - map back to C chars so buffer code can map back to LCD chars :(
//        map_to_c_chars(incoming_message, c_str, MAX_MSG_LENGTH, 0);
//
//        FbWriteWrappedLine(c_str);
//
//        if(msg_relay_remaining){
//            unsigned char tmp_relay[] = {'0' + (msg_relay_remaining/10)%10,
//                                         '0' + msg_relay_remaining%10,
//                                         0};
//            FbMove(100, 69);
//            FbColor(RED);
//            FbFilledRectangle(20, 10);
//
//            FbMove(103, 69);
//
//            FbColor(WHITE);
//            //FbCharacter(tmp_relay);
//            FbWriteLine(tmp_relay);
//        }
//
//        red(20);
//        green(100);
//        blue(20);
//        main_m[4].attrib = VERT_ITEM;
//    }
//    else if(G_menuStack[G_menuCnt].currMenu == main_m){
//        main_m[4].attrib = VERT_ITEM|LAST_ITEM|HIDDEN_ITEM;
//        red(0);
//        green(0);
//        blue(0);
//    }

    /* in case last menu item is a skip */
    if (selected == NULL) selected = root_menu;


    return (selected);
}

#ifndef SDL_BADGE
extern struct menu_t myBadgeid_m[];
extern struct menu_t peerBadgeid_m[];
extern struct menu_t backlight_m[];
extern struct menu_t rotate_m[];
extern struct menu_t LEDlight_m[];
extern struct menu_t buzzer_m[];
extern const struct menu_t sch_main_m[];



void returnToMenus(){
    TaskHandle_t xHandle = xTaskGetHandle("APP Tasks");
    BaseType_t notify_ret;
    if(xHandle == NULL)
        led(100, 0, 0);

    clear_buttons();
    notify_ret = xTaskNotify(xHandle,
                             1u,
                             eSetBits);

    vTaskSuspend(NULL);
}


//struct menu_t settings_m[] = {
////    {"QC", VERT_ITEM, TASK,
////        {hello_world_task}},
////    {"tutorial", VERT_ITEM, TASK,
////        {(struct menu_t *)badge_tutorial_task}},
////    {"ping", VERT_ITEM, FUNCTION,
////        {(struct menu_t *)ping_cb}},
//    {"backlight", VERT_ITEM, MENU,
//        {(struct menu_t *) backlight_m}},
////    {"led", VERT_ITEM, MENU,
////        {(struct menu_t *) LEDlight_m}}, /* coerce/cast to a menu_t data pointer */
//    {"buzzer", VERT_ITEM, MENU,
//        {(struct menu_t *) buzzer_m}},
//#ifdef IS_ADMIN
//        {"silence others", VERT_ITEM, TASK,
//                {(struct menu_t *) silence_task}},
//        {"full fucktard", VERT_ITEM, TASK,
//                {(struct menu_t *) note_crazy_task}},
//#endif
//    {"Back", VERT_ITEM | LAST_ITEM| DEFAULT_ITEM, BACK,
//        {NULL}},
//};
//
//struct menu_t gadgets_m[] = {    
////    {"Conductor", VERT_ITEM, TASK,
////        {conductor_task}},
////    {"blinkenlite", VERT_ITEM, TASK,
////        {blinkenlights_task}},        
////    {"dice roll", VERT_ITEM, TASK,
////        {dice_roll_task}},
//    {"u draw", VERT_ITEM, TASK,
//        {udraw_task}},
//    {"Back", VERT_ITEM | LAST_ITEM| DEFAULT_ITEM, BACK,
//        {NULL}},
//} ;        
//
//struct menu_t games_m[] = {
////    {"GroundWar", VERT_ITEM, TASK,
////        {groundwar_task}},    
////    {"Badgelandia", VERT_ITEM, TASK,
////        {badgelandia_task}},
////    {"Badgey Bird", VERT_ITEM, TASK,
////        {badgey_bird_task}},
//    {"Lander", VERT_ITEM, TASK,
//        {badge_lander_task}},
//    {"Star Shooter", VERT_ITEM, TASK,
//        {star_shooter_task}},
//    {"Back", VERT_ITEM | LAST_ITEM| DEFAULT_ITEM, BACK,
//        {NULL}},
//} ;


struct menu_t main_m[] = {
//    {"Arcade",       VERT_ITEM|DEFAULT_ITEM, MENU,
//        {games_m}},
//    {"Schedule", VERT_ITEM|DEFAULT_ITEM, MENU, {sch_main_m}},
//    {"GroundWar", VERT_ITEM, TASK, {groundwar_task}},        
//    {"Lander", VERT_ITEM, TASK, {badge_lander_task}},
//    {"U Draw", VERT_ITEM, TASK, {udraw_task}},    
    {"sensors", VERT_ITEM, TASK, (union menu_data_t) adc_task},
//    {"Conductor", VERT_ITEM, TASK, (union menu_data_t) conductor_task},
    {"Jukebox", VERT_ITEM|DEFAULT_ITEM, TASK, (union menu_data_t) jukebox_task},        

    {"blinkenlite", VERT_ITEM, TASK, (union menu_data_t) blinkenlights_task},        
//    {"Rubix", VERT_ITEM, TASK, {rubix_task}},
//    {"badgelandia", VERT_ITEM, TASK, {badgelandia_task}},

//    {"tutorial", VERT_ITEM, TASK,   {(struct menu_t *)badge_tutorial_task}},        
    {"backlight", VERT_ITEM, MENU, {(struct menu_t *) backlight_m}},
    
//    {"led", VERT_ITEM, MENU,
//        {(struct menu_t *) LEDlight_m}}, /* coerce/cast to a menu_t data pointer */
    
    {"buzzer", VERT_ITEM|LAST_ITEM, MENU, {(struct menu_t *) buzzer_m}},     
    
//    {"", VERT_ITEM|LAST_ITEM|HIDDEN_ITEM, BACK,
//        {NULL}},
};


#define TIME_BETWEEN_SCREENSAVER 2000
#define TIME_BEFORE_SLEEP 30000
//#define LAUNCH_APP groundwar_task
#define LAUNCH_APP boot_splash_task
//#define LAUNCH_APP badge_tutorial_task
//#define RUN_TUTORIAL 
//#define QC_FIRST
#define DO_BOOT_SPLASH
//#define QC_FIRST
//#define DO_BOOT_SPLASH
//#define DEBUG_PRINT_TO_CDC

// speed up flashing
enum badge_idle_state{
    AWAKE,
    ENTER_SLEEP,
    LOW_POWER_SLEEP,
    SCREENSAVER,
    WAIT_FOR_SCREENSAVER,
    WAKEUP
};

//#define NOSCREENSAVER
#ifndef NOSCREENSAVER
extern unsigned char stop_screensaver;
#endif

void menu_and_manage_task(void *p_arg){

#ifndef DONT_BLANK_MENU
    unsigned int sleep_timestamp = 0, screensaver_timestamp=0;
    TaskHandle_t xHandle = NULL;
    uint32_t ulNotifiedValue = 0;
    BaseType_t xReturned = 0;
    struct menu_t *prev_selected_menu = NULL;
    unsigned char idle_state = AWAKE;
    //led(100, 0, 0);
#ifndef NOSCREENSAVER
    stop_screensaver = 0;
#endif
    
    G_currMenu = main_m;
    G_selectedMenu = &main_m[0];
    
#ifdef QC_FIRST
    xReturned = xTaskCreate((TaskFunction_t) hello_world_task,
                            "exec_app",
                            128u, //may want to increase?
                            NULL,
                            1u,
                            &xHandle);
#else

#define DOLAUNCH    
#ifdef DOLAUNCH
    xReturned = xTaskCreate((TaskFunction_t) LAUNCH_APP,
                            "exec_app",
                            250u, //may want to increase?
                            NULL,
                            1u,
                            &xHandle);
#endif

#ifdef RUN_TUTORIAL
    // Wait for splash app to finish
    if (xTaskNotifyWait(0, 1u, &ulNotifiedValue, portMAX_DELAY)){
        if(ulNotifiedValue & 0x01){
            //vTaskSuspend(xHandle); // doesn't hurt to call suspend here too?
            vTaskDelete(xHandle);
            xHandle = NULL;
            prev_selected_menu = NULL;
            vTaskDelay(15 / portTICK_PERIOD_MS);
        }
    }    
    // TODO: Check flash for tutorial already complete. Allow access to tutorial in settings
    
    // Force tutorial
    xReturned = xTaskCreate((TaskFunction_t) badge_tutorial_task,
                            "exec_app",
                            250u, //may want to increase?
                            NULL,
                            1u,
                            &xHandle);    
#endif

#endif
    for(;;){
	static unsigned char fcnt=0;

	fcnt++;
//	flare_leds(fcnt >> 6);
        switch(idle_state){
#ifndef NOSCREENSAVER
            case AWAKE:
                // If enough time has past, turn off stuff and go to low power state
                if(TIME_SINCE_LAST_INPUT > TIME_BEFORE_SLEEP && xHandle == NULL){
                    idle_state = ENTER_SLEEP;
                }
                break;
            case ENTER_SLEEP:
                backlight(0);
                
                sleep_timestamp = timestamp;
                idle_state = LOW_POWER_SLEEP;
                break;
            case LOW_POWER_SLEEP:
                if(TIME_SINCE_LAST_INPUT < TIME_BEFORE_SLEEP){
                    idle_state = WAKEUP;
                }
                else if((timestamp - sleep_timestamp) > TIME_BETWEEN_SCREENSAVER){
                    idle_state = SCREENSAVER;
                }
                break;
            case SCREENSAVER:
                // Turn screen back on so we can see the screensaver!
                backlight(G_sysData.backlight);
                stop_screensaver = 0;
                xReturned = xTaskCreate((TaskFunction_t) random_screen_saver,
                                        "exec_app",
                                        350u, //may want to increase?
                                        NULL,
                                        1u,
                                        &xHandle);

                screensaver_timestamp = timestamp;

                idle_state = WAIT_FOR_SCREENSAVER;
                
                break;
            case WAIT_FOR_SCREENSAVER:
                // Check if the screen saver is ending itself
                if (xTaskNotifyWait(0, 1u, &ulNotifiedValue, 50 / portTICK_PERIOD_MS)){
                    if(ulNotifiedValue & 0x01){
                        vTaskSuspend(xHandle);
                        vTaskDelete(xHandle);
                        xHandle = NULL;
                        vTaskDelay(15 / portTICK_PERIOD_MS);
                        // RETURN to low power
                        idle_state = ENTER_SLEEP;
                    }
                }
                // Check if the user has provided any IO
                else if(DOWN_BTN_AND_CONSUME || UP_BTN_AND_CONSUME 
                        || LEFT_BTN_AND_CONSUME || RIGHT_BTN_AND_CONSUME){
                    //vTaskDelay(15 / portTICK_PERIOD_MS);
                    stop_screensaver = 1;
                    // screensaver must end
                    if (xTaskNotifyWait(0, 1u, &ulNotifiedValue, portMAX_DELAY)){
                        if(ulNotifiedValue & 0x01){
                            vTaskSuspend(xHandle);
                            vTaskDelete(xHandle);
                            xHandle = NULL;
                            vTaskDelay(15 / portTICK_PERIOD_MS);
                            // RETURN to low power
                            idle_state = ENTER_SLEEP;
                        }
                    }                    
                    
                    idle_state = WAKEUP;
                }
                // Screen saver has been running long enough...KILL!
                else if((timestamp - screensaver_timestamp) > TIME_BETWEEN_SCREENSAVER){
                    stop_screensaver = 1;
                    // screensaver must end
                    if (xTaskNotifyWait(0, 1u, &ulNotifiedValue, portMAX_DELAY)){
                        if(ulNotifiedValue & 0x01){
                            vTaskSuspend(xHandle);
                            vTaskDelete(xHandle);
                            xHandle = NULL;
                            vTaskDelay(15 / portTICK_PERIOD_MS);
                            // RETURN to low power
                            idle_state = ENTER_SLEEP;
                        }
                    }       

                    // RETURN to low power
                    idle_state = ENTER_SLEEP;
                }
                break;
#endif
            case WAKEUP:
                // turn on backlight and go to awake mode
                backlight(G_sysData.backlight);
                G_selectedMenu = display_menu(G_currMenu,
                              G_selectedMenu,
                              MAIN_MENU_STYLE);
                prev_selected_menu = G_selectedMenu;
                FbSwapBuffers();                
                setNote(90, 512);
                
                idle_state = AWAKE;
                break;
          }
        
        IRhandler();

        //Handle other events? Marshal messages?
#ifdef DEBUG_PRINT_TO_CDC
        // TODO: this will have a conflict in the notify field with the kill signal
        print_high_water_marks();
        vTaskDelay(200 / portTICK_PERIOD_MS);
#else
        // Doesn't need to be too responsive
        vTaskDelay(5 / portTICK_PERIOD_MS);
#endif        
        if(idle_state != AWAKE)
            continue;
        
        // No running task and badge is awake -> display menu or something
        if(xHandle == NULL && idle_state == AWAKE)
        {
           //led(30, 30, 0);
            // Only redraw when we must
            if (prev_selected_menu != G_selectedMenu){
                G_selectedMenu = display_menu(G_currMenu,
                                              G_selectedMenu,
                                              MAIN_MENU_STYLE);
                prev_selected_menu = G_selectedMenu;
//                FbMove(1, 100);
//                FbImage(RVASEC2016);
                FbSwapBuffers();
            }

            if (BUTTON_PRESSED_AND_CONSUME){//BUTTON_PRESSED_AND_CONSUME) {
                // action happened that will result in menu redraw
                //do_animation = 1;
                switch (G_selectedMenu->type) {
//                    case MORE: /* jump to next page of menu */
//                        setNote(173, 2048); /* a */
//                        G_currMenu += PAGESIZE;
//                        G_selectedMenu = G_currMenu;
//                        break;
                    case BACK: /* return from menu */
                        //setNote(154, 2048);
                        if (G_menuCnt == 0) return; /* stack is empty, error or main menu */
                        G_menuCnt--;
                        G_currMenu = G_menuStack[G_menuCnt].currMenu;
                        G_selectedMenu = G_menuStack[G_menuCnt].selectedMenu;
                        //G_selectedMenu = G_currMenu;
                        break;

                    case TEXT: /* maybe highlight if clicked?? */
                        //setNote(145, 2048); /* c */
                        break;

                    case MENU: /* drills down into menu if clicked */
                        //setNote(129, 2048); /* d */
                        G_menuStack[G_menuCnt].currMenu = G_currMenu; /* push onto stack  */
                        G_menuStack[G_menuCnt].selectedMenu = G_selectedMenu;
                        G_menuCnt++;
                        if (G_menuCnt == MAX_MENU_DEPTH) G_menuCnt--; /* too deep, undo */
                        G_currMenu = (struct menu_t *) G_selectedMenu->data.menu; /* go into this menu */
                        //selectedMenu = G_currMenu;
                        G_selectedMenu = NULL;
                        break;
						
					case SMENU: /* Special case of MENU for schedule nav */
                        //setNote(129, 2048); /* d */
                        G_menuStack[G_menuCnt].currMenu = G_currMenu; /* push onto stack  */
                        G_menuStack[G_menuCnt].selectedMenu = NULL; /* Reset Cursor */
                        G_menuCnt++;
                        if (G_menuCnt == MAX_MENU_DEPTH) G_menuCnt--; /* too deep, undo */
                        G_currMenu = (struct menu_t *) G_selectedMenu->data.menu; /* go into this menu */
                        //selectedMenu = G_currMenu;
                        G_selectedMenu = NULL;
                        break;

                    case TASK: // Launch a task if clicked
                        setNote(105, 2048);
                        xReturned = xTaskCreate((TaskFunction_t) G_selectedMenu->data.task, 
                                                "exec_app",
                                                550u, //may want to increase?
                                                NULL,
                                                1u,
                                                &xHandle);                        
                        break;
                    case FUNCTION: /* call the function pointer if clicked */
                        setNote(115, 2048); /* e */
                        (*G_selectedMenu->data.func)();
                        break;
                    default:
                        break;
                }
            }
            else if (UP_TOUCH_AND_CONSUME || UP_BTN_AND_CONSUME) /* handle slider/soft button clicks */ {
                setNote(109, 800); /* f */

                /* make sure not on first menu item */
                if (G_selectedMenu > G_currMenu) {
                    G_selectedMenu--;

                    while (((G_selectedMenu->attrib & SKIP_ITEM) || (G_selectedMenu->attrib & HIDDEN_ITEM))
                            && G_selectedMenu > G_currMenu)
                        G_selectedMenu--;

                    //G_selectedMenu = display_menu(G_currMenu, G_selectedMenu, MAIN_MENU_STYLE);
                }
            }/* *** PEB ***** not convinced this should be an else
               both sliders can be pressed then this one will never get handled
            */
            else if (DOWN_TOUCH_AND_CONSUME || DOWN_BTN_AND_CONSUME) {
                setNote(97, 1024); /* g */

                /* make sure not on last menu item */
                if (!(G_selectedMenu->attrib & LAST_ITEM)) {
                    G_selectedMenu++;

                    //Last item should never be a skipped item!!
                    while (((G_selectedMenu->attrib & SKIP_ITEM) || (G_selectedMenu->attrib & HIDDEN_ITEM))
                            && (!(G_selectedMenu->attrib & LAST_ITEM)))
                        G_selectedMenu++;

                    // at this point, may be on last item, if it's hidden, back off of it
                    if((G_selectedMenu->attrib & LAST_ITEM) && (G_selectedMenu->attrib & HIDDEN_ITEM))
                        G_selectedMenu--;

                   // G_selectedMenu = display_menu(G_currMenu, G_selectedMenu, MAIN_MENU_STYLE);
                }
            }
        }
        // Manage a running task
        else if(xHandle !=NULL  && idle_state == AWAKE){
            // TODO: Might be able to let task kill itself
            if (xTaskNotifyWait(0, 1u, &ulNotifiedValue, 50 / portTICK_PERIOD_MS)){
                if(ulNotifiedValue & 0x01){
                    //vTaskSuspend(xHandle); // doesn't hurt to call suspend here too?
                    vTaskDelete(xHandle);
                    xHandle = NULL;
                    prev_selected_menu = NULL;   
                }
            }
        }
    }
#endif
}
#endif
