#include "app.h"
#include "queue.h"
#include "buttons.h"
#include "ir.h"
#include "flash.h"
// badge
#include "colors.h"
#include "fb.h"
#include "rgb_led.h"
#include "task.h"
#include "badge_menu.h"
#include "assetList.h"
#include "adc.h"

extern unsigned char QC_IR;

#define LED_LVL 50
void QC(void* p_arg)
{
    //static unsigned char call_count = 0;
    const TickType_t xDelay = 20 / portTICK_PERIOD_MS;
    BaseType_t notify_ret;
    //TaskHandle_t xHandle = xTaskGetHandle("APP Tasks");
    unsigned char redraw = 0;
    union IRpacket_u pkt;
    pkt.p.command = IR_WRITE;
    //pkt.p.address = IR_LED;
    pkt.p.address = IR_APP0;
    pkt.p.badgeId = 0;
    //pkt.p.data    = PING_PAIR_REQUEST;
    pkt.p.data = 1;//PACKRGB(0, 100, 100);
    
    FbTransparentIndex(0);
    FbColor(GREEN);
    FbClear();

    FbMove(45, 45);
    FbWriteLine("QC!");
    FbMove(15, 55);
    FbWriteLine("Do things");
    FbSwapBuffers();
    flare_leds(200);
    led(0, 30, 0);
    QC_IR = 0;
    
    char adc_on = 0;
    unsigned short mic_adc=0;
    char words[8]={'.', '\n', '\0', 0, 0, 0, 0, 0};
    
    for(;;)
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
        
        // Received a QC ping
        if(QC_IR == 1){
            setNote(77, 1024);
            FbMove(10, 50);
            FbColor(GREEN);
            //FbFilledRectangle(20, 20);
            FbWriteLine("I was pinged");
            led(100, 0, 100); 
            QC_IR = 0;
            redraw = 1;
            pkt.p.data = 2;
            IRqueueSend(pkt);
        }
        // Received a QC ping 
        else if(QC_IR == 2){
            setNote(99, 2048);
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

        if((G_touch_pct >0) && !(adc_on) ){
            FbMove(16, 16);
            //FbWriteLine((unsigned char) "TOUCH");
            FbWriteLine("TOUCH");
            //print_to_com1("TOUCH\n\r");
                       
            FbMove(100, 132 - (G_touch_pct + 10));
            FbColor(WHITE);
            FbFilledRectangle(5, 15);
            redraw = 1;
            led(0, 0, 0);
            setNote(100 - (G_touch_pct), 1024);
        }      

        if(DOWN_BTN_AND_CONSUME){
            FbMove(16, 16);
            FbWriteLine("DOWN");
            setNote(55, 1024);
            led(0, LED_LVL, 0);
            //print_to_com1("DOWN\n\r");
            redraw = 1;
        }
        
        if(G_button_cnt > 100){
            FbMove(16, 26);
            FbWriteLine("EXITING");
            FbSwapBuffers();
            led(0,0,0);
            vTaskDelay(xDelay);
            returnToMenus();
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
        
        if(DOWN_BTN_HOLD(155)){
            if(!adc_on){
                adc_on = 1;
                //ADC_init(0, 2); // Init all
                ADC_init(2, 2); // Init just mic
                setNote(20, 1024);
            }else{
//                adc_on = 0;
//                setNote(60, 1024);
            }
        }
        
        if (adc_on){           

#define MIC_CHANNEL 0
#define ANTENNA_CHANNEL 2            
#define N_CHANNELS 1
            if(ADCbufferCntMark == 0){
                unsigned char i = 0;
                if(ADCbufferCnt >= ADC_BUFFER_SIZE ){
                    
                    //for(i=0; i < ADC_BUFFER_SIZE; i+=N_CHANNELS){
                    //for(i=0; i < 128; i+=N_CHANNELS){
                    for(i=0; i < 120; i+=N_CHANNELS){
                        //------ MIC
                        FbColor(GREEN);
                        mic_adc = ADCbuffer[i + MIC_CHANNEL ] >> 1;
                        
                        mic_adc = mic_adc < 64 ? mic_adc : 64;
                        FbMove(i + 1, 120 - (unsigned char) (mic_adc));
                        FbFilledRectangle(2, 2);
                        
                        //----- RF
//                        FbColor(YELLOW);
//                        mic_adc = ADCbuffer[i + ANTENNA_CHANNEL ];
//                        
//                        mic_adc = mic_adc < 64 ? mic_adc : 64;
//                        FbMove(i, 70 - (unsigned char) (mic_adc));
//                        FbFilledRectangle(2, 2);
                    }// END LOOP
                    
                    redraw=1;
                    ADCbufferCntMark = 1;
                }                
            }
        }
        
        if(redraw){
            redraw = 0;
            FbSwapBuffers(); 
        }
        
        vTaskDelay(xDelay);

    }
    vTaskDelete( NULL );
}
