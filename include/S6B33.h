/** \file
 * Low-level functions for updating the LCD display.
 */
#ifndef S6B33_h
#define S6B33_h

/** set important internal registers for the LCD display */
void S6B33_init_device(void);
/** sets the current display region for calls to S6B33_pixel() */
void S6B33_rect(int x, int y, int width, int height);
/** updates current pixel to the data in `pixel`. */
void S6B33_pixel(unsigned short pixel);

#endif
