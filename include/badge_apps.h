/* 
 * File:   badge_apps.h
 * Author: morgan
 *
 * Created on March 4, 2017, 6:33 PM
 */

// Create one header that declares everything needed to create/use
// badge application tasks

#ifndef BADGE_APPS_H
#define	BADGE_APPS_H

// Declare tasks
void adc_task(void* p_arg);

void QC(void* p_arg);

void badgey_bird_task(void* p_arg);

void badgelandia_task(void* p_arg);

void groundwar_task(void* p_arg);

void screensaver_task(void* p_arg);

void boot_splash_task(void *p_arg);

void blinkenlights_task(void *args);

void conductor_task(void *args);

void dice_roll_task(void* p_arg);

void badge_tutorial_task(void* p_arg);

void star_shooter_task(void* p_arg);

void udraw_task(void* p_arg);

void random_screen_saver(void* p_arg);

void silence_task(void *p_arg);

void note_crazy_task(void *p_arg);

void badge_lander_task(void *p_arg);

void rubix_task(void *p_arg);

void jukebox_task(void *p_arg);
#endif

