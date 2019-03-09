#include "bindings.h"

/*

cc -m32 -DMAIN -I./include -o bindings src/bindings.c
./bindings


*/

#ifndef MAIN

#include "flash.h"
#include "buttons.h"
#include "timer1_int.h"
#include "assets.h"
#include "assetList.h"

// keep in sync with .h
const char *ptrType[] = {
    "var",
    "func"
};

// keep in sync with .h
const char *parmType[] = {
    "void",
    "char",
    "char*",
    "int",
    "int*",
};

struct binding_t bindings[] = {
    {.vf.cp  = (char *)&G_button_cnt},
    {.vf.cp  = (char *)&G_up_button_cnt},
    {.vf.cp  = (char *)&G_down_button_cnt},
    {.vf.cp  = (char *)&G_left_button_cnt},
    {.vf.cp  = (char *)&G_right_button_cnt},
    {.vf.cp  = (char *)&(wclock.hour)},
    {.vf.cp  = (char *)&(wclock.min)},
    {.vf.cp  = (char *)&(wclock.sec)},
    {.vf.ip  = (int *)&G_sysData.badgeId},
    {.vf.cp  = (char *)G_sysData.name},
    {.vf.ip  = (int *)&flashedBadgeId},
    {.vf.ip  = (int *)&G_flashAddr},
    {.vf.fun = (drawLCD8)},
    {.vf.c   = (char)DRBOB},
};

/*
    first is for function/variable, second for return type, third...end for parm types
    parms types are used for listing function
*/
struct param_t params[] = {
    { "MAINBUTTON",	B_VARIABLE, B_CHAR,      B_VOID,     B_VOID,  B_VOID,  B_VOID,  B_VOID,  B_VOID,  B_VOID },
    { "UPBUTTON",	B_VARIABLE, B_CHAR,      B_VOID,     B_VOID,  B_VOID,  B_VOID,  B_VOID,  B_VOID,  B_VOID },
    { "DOWNBUTTON",	B_VARIABLE, B_CHAR,      B_VOID,     B_VOID,  B_VOID,  B_VOID,  B_VOID,  B_VOID,  B_VOID },
    { "LEFTBUTTON",	B_VARIABLE, B_CHAR,      B_VOID,     B_VOID,  B_VOID,  B_VOID,  B_VOID,  B_VOID,  B_VOID },
    { "RIGHTBUTTON",	B_VARIABLE, B_CHAR,      B_VOID,     B_VOID,  B_VOID,  B_VOID,  B_VOID,  B_VOID,  B_VOID },
    { "HOUR", 		B_VARIABLE, B_CHAR,      B_VOID,     B_VOID,  B_VOID,  B_VOID,  B_VOID,  B_VOID,  B_VOID },
    { "MIN", 		B_VARIABLE, B_CHAR,      B_VOID,     B_VOID,  B_VOID,  B_VOID,  B_VOID,  B_VOID,  B_VOID },
    { "SEC", 		B_VARIABLE, B_CHAR,      B_VOID,     B_VOID,  B_VOID,  B_VOID,  B_VOID,  B_VOID,  B_VOID },
    { "BADGEID", 	B_VARIABLE, B_INT,       B_VOID,     B_VOID,  B_VOID,  B_VOID,  B_VOID,  B_VOID,  B_VOID },
    { "NAME", 		B_VARIABLE, B_CHARSTAR,  B_VOID,     B_VOID,  B_VOID,  B_VOID,  B_VOID,  B_VOID,  B_VOID },
    { "FLASHEDID", 	B_VARIABLE, B_INT,       B_VOID,     B_VOID,  B_VOID,  B_VOID,  B_VOID,  B_VOID,  B_VOID },
    { "FLASHADDR", 	B_VARIABLE, B_INT,       B_VOID,     B_VOID,  B_VOID,  B_VOID,  B_VOID,  B_VOID,  B_VOID },
    { "DRAWLCD8", 	B_FUNCTION, B_VOID,      B_CHAR,     B_INT,   B_VOID,  B_VOID,  B_VOID,  B_VOID,  B_VOID },
    { "DRBOB", 		B_VARIABLE, B_CHARSTAR,  B_VOID,     B_VOID,  B_VOID,  B_VOID,  B_VOID,  B_VOID,  B_VOID },
};

enum {
    B_VAR,
    B_FUNC
};

#ifdef MAIN
#include <stdio.h>

struct wallclock_t {
   unsigned char hour;
   unsigned char sec;
   unsigned char min;

   unsigned int now;
   unsigned int last;
   unsigned int delta;
   unsigned int accum;
};

struct sysData_t {
   char name[32];
   unsigned short badgeId; /* 2 bytes == our badge Id */
   char sekrits[8];
   char achievements[8];

   /*
      prefs
   */
   char ledBrightness;  /* 1 byte */
   char backlight;      /* 1 byte */
};

struct sysData_t G_sysData;
struct wallclock_t wclock;

char G_button_cnt;
char G_up_button_cnt;
char G_down_button_cnt;
char G_left_button_cnt;
char G_right_button_cnt;
int flashedBadgeId;
int *G_flashAddr;
void drawLCD8(unsigned char , int);
char drbob[32];

void listbindings()
{
    int i;

    for (i=0; i< LASTBINDING; i++) {
	if ( params[i].var_fun == B_FUNCTION ) {
	    printf("%s %s(", parmType[params[i].ret], params[i].name);

	    if (params[i].arg0 != B_VOID) {
	        printf("%s", parmType[params[i].arg0]);

	        if (params[i].arg1 != B_VOID) {
	            printf(", %s", parmType[params[i].arg1]);

	            if (params[i].arg2 != B_VOID) {
	                printf(", %s", parmType[params[i].arg2]);

			if (params[i].arg3 != B_VOID) {
	                    printf(", %s", parmType[params[i].arg3]);

			    if (params[i].arg4 != B_VOID) {
	                        printf(", %s", parmType[params[i].arg4]);

			        if (params[i].arg5 != B_VOID) {
	                            printf(", %s", parmType[params[i].arg5]);

			            if (params[i].arg6 != B_VOID) {
	                                printf(", %s", parmType[params[i].arg6]);
			            }
			        }
			    }
			}
		    }
		}
	    }

	    printf(")\n");
	}
	else
	if ( params[i].var_fun == B_VARIABLE ) {
	    printf("%s %s\n", parmType[params[i].ret], params[i].name);
	}
    }
}

void drawLCD8(unsigned char assetId, int frame)
{

}

int main(int argc, char *argv[])
{
    char s[] = "hopadoop";


    printf("sizeof fun_var %d\n", sizeof(union var_fun));
    printf("sizeof binding %d\n", sizeof(struct binding_t));
    listbindings();


    bindings[DRAWLCD8].vf.fun(s, 0, 1, 2, 3, 4, 5, 6);

    return(0);
}
#endif
