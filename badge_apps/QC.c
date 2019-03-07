#include "buttons.h"
#include "ir.h"
#include "colors.h"
#include "fb.h"
#include "badge_menu.h"

extern unsigned char QC_IR;

enum { 
   INIT,
   RUN
};


#define LED_LVL 50
void QC_cb()
{
    //static unsigned char call_count = 0;
    static int QC_state=0;
    unsigned char redraw = 0;
    union IRpacket_u pkt;
    pkt.p.command = IR_WRITE;
    //pkt.p.address = IR_LED;
    pkt.p.address = IR_APP0;
    pkt.p.badgeId = 0;
    //pkt.p.data    = PING_PAIR_REQUEST;
    pkt.p.data = 1;//PACKRGB(0, 100, 100);
    
   
    switch(QC_state)
    {
//        if(pinged){
//            setNote(77, 1024);
//            FbClear();
//            FbMove(40, 50);
//            FbColor(GREEN);
//            //FbFilledRectangle(20, 20);
//            FbWriteLine('IR RECV');
//            setNote(50, 2048);
//            led(100, 0, 100); 
//            pinged = 0;
//            redraw = 1;
//        }
	case INIT:
	    FbTransparentIndex(0);
	    FbColor(GREEN);
	    FbClear();

	    FbMove(45, 45);
	    FbWriteLine("QC!");
	    FbMove(15, 55);
	    FbWriteLine("Do things");
	    FbSwapBuffers();
	    led(0, 30, 0);
	    QC_IR = 0;
	    QC_state++;
            redraw = 1;
	    break;
         
	case RUN:
        // Received a QC ping
        if(QC_IR == 1){
            setNote(80, 4096);
            FbMove(10, 40);
            FbColor(GREEN);
            //FbFilledRectangle(20, 20);
            FbWriteLine("I was pinged");
            led(100, 0, 100); 
            QC_IR = 0;
            pkt.p.data = 2;
            IRqueueSend(pkt);
            redraw = 1;
        }
        // Received a QC ping 
        else if(QC_IR == 2){
            setNote(60, 2048);
            FbMove(10, 50);
            FbColor(YELLOW);
            //FbFilledRectangle(20, 20);
            //FbWriteLine('IR RECV');
            FbWriteLine("ping response");
            led(100, 100, 0); 
            QC_IR = 0;
            redraw = 1;          
        }
        
        if(BUTTON_PRESSED_AND_CONSUME){          
            pkt.p.data = 1;
            IRqueueSend(pkt);
            setNote(50, 1024);
            FbMove(16, 16);
            FbWriteLine("BTN");
            led(LED_LVL, 0, LED_LVL);
            //print_to_com1("DOWN\n\r");
            redraw = 1;
        }

        if(DOWN_BTN_AND_CONSUME){
            FbMove(16, 16);
            FbWriteLine("DOWN");
            setNote(55, 1024);
            led(0, LED_LVL, 0);
            //print_to_com1("DOWN\n\r");
            redraw = 1;
        }
        
        if(G_button_cnt > 200){
            FbMove(16, 26);
            FbWriteLine("EXITING");
            FbSwapBuffers();
            led(0,0,0);
            QC_state = 0;
            returnToMenus();
	    return;
        }
        
        if(UP_BTN_AND_CONSUME){
            FbMove(16, 16);
            FbWriteLine("UP");
            setNote(60, 1024);
            led(LED_LVL, LED_LVL, LED_LVL);
            //print_to_com1("UP\n\r");
            redraw = 1;
        }

        if(LEFT_BTN_AND_CONSUME){
            FbMove(16, 16);
            FbWriteLine("LEFT");
            setNote(45, 1024);
            led(LED_LVL, 0, 0);
            //print_to_com1("LEFT\n\r");
            redraw = 1;
        }
        
        if(RIGHT_BTN_AND_CONSUME){
            FbMove(16, 16);
            FbWriteLine("RIGHT");
            led(0, 0, LED_LVL);
            setNote(20, 1024);
            //print_to_com1("RIGHT");
            redraw = 1;
        }        
        
        if(redraw){
            redraw = 0;
            FbSwapBuffers(); 
        }
    }
}
