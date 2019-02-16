/* 
   peb 20180228
   ADC constants and structs
*/

#ifndef ADC_H
#define	ADC_H

enum ANALOG_SOURCES {
   AN_MIC,
   AN_RED_GREEN_BLUE,
   AN_RED,
   AN_GREEN,
   AN_BLUE,
   AN_VSS,
   AN_LAST
};

struct analog_src_t {
   char name[16];    // human name
   unsigned short ANmask; // single or combo bit mask from below
   unsigned short chans;  // # channels if a combo
} analog_src_t;

/* 
   these are used for AD1CSSL the ADC input scan select register

   CSSL = ANx, where ‘x’ = 0-12; 
   CSSL13 selects CTMU input for scan; 
   CSSL14 selects IV REF for scan; - current ref
   CSSL15 selects VSS for scan.
*/
#define AN_VSS_MASK     0b1000000000000000
#define AN_IREF_MASK    0b0100000000000000
#define AN_CTMU_MASK    0b0010000000000000
#define AN_MIC_MASK     0b0001000000000000 /* AN12 */
#define AN_HDR2_MASK    0b0000100000000000 /* AN11 second empty pin on header */
#define AN_BTN_MASK     0b0000010000000000 /* AN10 main button input */
#define AN_IRIN_MASK    0b0000001000000000 /* AN9 IR input */
#define AN_RDPAD_MASK   0b0000000100000000 /* AN8 right d-pad button */
#define AN_BLUE_MASK    0b0000000010000000 /* AN7 B */
#define AN_RED_MASK     0b0000000001000000 /* AN6 R */
#define AN_GREEN_MASK   0b0000000000100000 /* AN5 G */
#define AN_UDPAD_MASK   0b0000000000010000 /* AN4 up d-pad button */
#define AN_DDPAD_MASK   0b0000000000001000 /* AN3 down d-pad button */
#define AN_LDPAD_MASK   0b0000000000000100 /* AN2 left d-pad button */
#define AN_IROUT_MASK   0b0000000000000010 /* AN1 IR-output button */
#define AN_HDR1_MASK    0b0000000000000001 /* AN0 1st empty pin on header  */


/* 
   needs to be an multiple of the pins being sampled
   so multiples fit in the buffer 1/2/4
*/

#define MAX_ADC_PINS 4

// one LCD scanline 
#define ADC_SAMPLES_PER_LINE 128

// one scanline of samples if all 4 pins sampling
#define ADC_BUFFER_SIZE (ADC_SAMPLES_PER_LINE * MAX_ADC_PINS)

void ADC_init(unsigned char analog_src_num, unsigned char hz_num); // use analog src enum and and hz enum {HZ_500 ... (HZ_LAST-1)}

struct sample_info_t {
   int hz; // see list below
   char name[8]; // human name
} sample_info_t;

enum SAMPLE_HZ {
   HZ_150,
   HZ_500,
   HZ_1000,
   HZ_2000,
   HZ_4000,
   HZ_8000,
   HZ_16000,
   HZ_32000,
   HZ_64000,
   HZ_96000,
   HZ_100000,
   HZ_125000,
   HZ_250000,
   HZ_LAST
};

// enum {HZ_500 ... (HZ_LAST-1)}
void ADC_init(unsigned char analog_src_num, unsigned char hz_num);
void ADC_stop();
void adc_cb();

extern const struct sample_info_t samples_info[];
extern const struct analog_src_t analog_info[] ;

extern volatile unsigned short ADCbuffer[];
extern volatile unsigned int ADCbufferCnt;
extern volatile unsigned int ADCbufferCntMark;
extern const volatile unsigned int *ADCbufferAddresses[][8];

extern volatile int G_adc_sum;
extern volatile int G_adc_samps;
extern volatile short G_adc_sum_done;

 
#define AUDIO_PHASE1 LATAbits.LATA9
#define AUDIO_PHASE2 LATAbits.LATA4

#endif	/* ADC_H */

