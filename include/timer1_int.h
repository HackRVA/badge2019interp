#ifndef timer1_int_h
#define timer1_int_h

struct wallclock_t {
   unsigned char low;
   unsigned char hi;
   unsigned char hour;
   unsigned char sec;
   unsigned char min;

   unsigned int now;
   unsigned int last;
   unsigned int lag;
   unsigned int delta;
};

extern struct wallclock_t wclock;

void red(unsigned char onPWM) ;
void green(unsigned char onPWM) ;
void blue(unsigned char onPWM) ;
void led(unsigned char r, unsigned char g, unsigned char b);
void backlight(unsigned char bright) ;
void led_input_hack(unsigned char OnOff) ; // use LEDs as inputs
void no_LED_PWM(unsigned char onOff) ;

void timerInit();

#endif

