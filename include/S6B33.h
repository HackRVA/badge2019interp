/** \file
 * Low-level functions for updating the LCD display.
 */
#ifndef S6B33_h
#define S6B33_h

/* controller commands- S6B33B2 */
#define NOP                  0x00
#define OSCILLATION_MODE     0x02
#define DRIVER_OUTPUT_MODE   0x10
#define DCDC_SELECT          0x20
#define BIAS_SET             0x22
#define DCDC_CLOCK_DIV       0x24
#define DCDC_AMP_ONOFF       0x26
#define TEMP_COMPENSATION    0x28
#define CONTRAST_CONTROL1    0x2A
#define CONTRAST_CONTROL2    0x2B
#define STANDBY_OFF          0x2C
#define STANDBY_ON           0x2D
#define DDRAM_BURST_OFF      0x2E
#define DDRAM_BURST_ON       0x2F
#define ADDRESSING_MODE      0x30
#define ROW_VECTOR_MODE      0x32
#define N_LINE_INVERSION     0x34
#define FRAME_FREQ_CONTROL   0x36
#define RED_PALETTE          0x38
#define GREEN_PALETTE        0x3A
#define BLUE_PALETTE         0x3C
#define ENTRY_MODE           0x40
#define X_ADDR_AREA          0x42
#define Y_ADDR_AREA          0x43
#define RAM_SKIP_AREA        0x45
#define DISPLAY_OFF          0x50
#define DISPLAY_ON           0x51
#define SPEC_DISPLAY_PATTERN 0x53
#define PARTIAL_DISPLAY_MODE 0x55
#define PARTIAL_START_LINE   0x56
#define PARTIAL_END_LINE     0x57
#define AREA_SCROLL_MODE     0x59
#define SCROLL_START_LINE    0x5A
#define DATA_FORMAT_4K_XRGB  0x60
#define DATA_FORMAT_4K_RGBX  0x61
#define OTP_MODEOFF          0xEA

/* display mode settings for conditionals */
#define DISPLAY_MODE_NORMAL 0b00000110
#define DISPLAY_MODE_INVERTED 0b00000011

/** set important internal registers for the LCD display */
void S6B33_init_device(void);
/** sets the current display region for calls to S6B33_pixel() */
void S6B33_rect(int x, int y, int width, int height);
/** updates current pixel to the data in `pixel`. */
void S6B33_pixel(unsigned short pixel);
/** invert display */
void S6B33_set_display_mode_inverted(void);
/** uninvert display */
void S6B33_set_display_mode_noninverted(void);
/** get current display mode variable setting */
unsigned char S6B33_get_display_mode(void);

#endif
