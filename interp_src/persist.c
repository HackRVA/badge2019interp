// to stop persist() do:
//   new <enter>
//   run <enter>
//
// in usb/serial terminal
// this emptys the src/interp

// if this gets too big may have to setAlloc() & run

// badge bindings. see badge.c bindings[] 
enum {
    MAINBUTTON, UPBUTTON, DOWNBUTTON, LEFTBUTTON, RIGHTBUTTON,
    WALLHOUR, WALLMIN, WALLSEC,
    BADGEID, NAME, FLASHEDBADGEID, FLASHADDR,
    DRAWLCD8, DRBOB, 
};

int i;
int persist(int argc, int *argv)
{
   char *mainbtn, *upbtn, *downbtn;
   int *fbadgeid;

   // badge functions args types and here have to be the same
   // because of int <-> char alignment otherwise will fault and lockup badge
   // btns are unsigned char types but interp doesn't have "unsigned"
   mainbtn = (char *)argv[MAINBUTTON];
   upbtn = (char *)argv[UPBUTTON];
   downbtn = (char *)argv[DOWNBUTTON];
   fbadgeid = (char *)argv[FLASHEDBADGEID];

   i = i + 1;
   i = i % 5000;

   if (i == 1) printx(*fbadgeid);
   if (i == 500) *downbtn = 200;
   if (i == 2500) *mainbtn = 200; // click on things randomly
   if (i == 4500) *upbtn = 200;

   // pretty lights
   if (i<1000) led((char)i, 0, 0);
   else 
   if (i<2000) led(0, (char)i, 0);
   else 
   if (i<3000) led(0, 0, (char)i);
   else 
   if (i<4000) led((char)i, (char)i, (char)i);

   return 0;
}


//
// example 2
// 

// variable AND callback functions: 
// drawlcd8 and parameters (DRBOB)
// get time 2 different ways
enum {
    MAINBUTTON, UPBUTTON, DOWNBUTTON, LEFTBUTTON, RIGHTBUTTON,
    WALLHOUR, WALLMIN, WALLSEC,
    BADGEID, NAME, FLASHEDBADGEID, FLASHADDR,
    DRAWLCD8, DRBOB, 
};

int i;
int persist(int argc, int *argv)
{
    char *hour, *min, *sec;

    // only do this once
    if (i==1) callback2((int)argv[DRAWLCD8], (char)argv[DRBOB], (int)0);

    // ditto
    if (i==2) {
	hour = (char *)argv[WALLHOUR]; // addresses of running time var
	min = (char *)argv[WALLMIN];
	sec = (char *)argv[WALLSEC];

	print("\n");
	printd((int)*hour);
	printd((int)*min);
	printd((int)*sec);

	print(getTime()); // interp built-in function call
    }
    i++;
}

