//
// keep this in sync with include/bindings.h
//
enum {
  MAINBUTTON, UPBUTTON, DOWNBUTTON, LEFTBUTTON, RIGHTBUTTON,
  HOUR, MIN, SEC,
  BADGEID, NAME, FLASHEDID, FLASHADDR,
  DRAWLCD8, DRBOB_BIND,
};

int i;
int persist(int argc, int *argv)
{
   char *mainbtn, *upbtn, *downbtn;
   int *fbadgeid;

   mainbtn = (char *)argv[MAINBUTTON];
   upbtn = (char *)argv[UPBUTTON];
   downbtn = (char *)argv[DOWNBUTTON];
   fbadgeid = (char *)argv[FLASHEDID];

   i = i + 1;
   i = i % 5000;

//   if (i == 1) printx(*fbadgeid);
   if (i == 500) *downbtn = 200;
   if (i == 2500) *mainbtn = 200; // clicks on things
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
  HOUR, MIN, SEC,
  BADGEID, NAME, FLASHEDID, FLASHADDR,
  DRAWLCD8, DRBOB,
};

int i;
int persist(int argc, int *argv)
{
    char *hour, *min, *sec;

    // ditto
    if (i==2) {
	hour = (char *)argv[HOUR]; // addresses of running time var
	min = (char *)argv[MIN];
	sec = (char *)argv[SEC];

	print("\n");
	printd((int)*hour);
	printd((int)*min);
	printd((int)*sec);

	print(getTime()); // interp built-in function call
    }
    i++;
}

//
// a call to a function
//
enum {
  MAINBUTTON, UPBUTTON, DOWNBUTTON, LEFTBUTTON, RIGHTBUTTON,
  HOUR, MIN, SEC,
  BADGEID, NAME, FLASHEDID, FLASHADDR,
  DRAWLCD8, DRBOB,
};

int i;
int persist(int argc, int *argv)
{
    // callback2 takes 2 parms.
    if (i==1) callback2((int)argv[DRAWLCD8], (char)argv[DRBOB], (int)0);

    i++;
}

