//
// keep this in sync with include/bindings.h
//
enum {
  MAINBTN, UPBTN, DOWNBTN, LEFTBTN, RIGHTBTN,
  HOUR, MIN, SEC,
  BADGEID, NAME, FL_ID, FL_ADDR,
  DRAWLCD8, DRBOB, ADC, ROTATE, 
  LEDBRIGHT, MUTE,
};

int i;
int persist(int argc, int *argv)
{
   char *mainbtn, *upbtn, *downbtn;
   int *fbadgeid;

   mainbtn = (char *)argv[MAINBTN];
   upbtn = (char *)argv[UPBTN];
   downbtn = (char *)argv[DOWNBTN];
   fbadgeid = (char *)argv[FL_ID];

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
  MAINBTN, UPBTN, DOWNBTN, LEFTBTN, RIGHTBTN,
  HOUR, MIN, SEC,
  BADGEID, NAME, FL_ID, FL_ADDR,
  DRAWLCD8, DRBOB, ADC, ROTATE, 
  LEDBRIGHT, MUTE,
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
  MAINBTN, UPBTN, DOWNBTN, LEFTBTN, RIGHTBTN,
  HOUR, MIN, SEC,
  BADGEID, NAME, FL_ID, FL_ADDR,
  DRAWLCD8, DRBOB, ADC, ROTATE, 
  LEDBRIGHT, MUTE,
};

int i;
int persist(int argc, int *argv)
{
    // callback2 takes 2 parms.
    if (i==1) callback2((int)argv[DRAWLCD8], (char)argv[DRBOB], (int)0);

    i++;
}

enum {
  MAINBTN, UPBTN, DOWNBTN, LEFTBTN, RIGHTBTN,
  HOUR, MIN, SEC,
  BADGEID, NAME, FL_ID, FL_ADDR,
  DRAWLCD8, DRBOB, ADC, ROTATE, 
  LEDBRIGHT, MUTE,
};

int i;
int persist(int argc, int *argv)
{
    // callback2 takes 2 parms.
    callback0((int)argv[ADC_CB]);
}

enum {
  MAINBTN, UPBTN, DOWNBTN, LEFTBTN, RIGHTBTN,
  HOUR, MIN, SEC,
  BADGEID, NAME, FL_ID, FL_ADDR,
  DRAWLCD8, DRBOB, ADC, ROTATE, 
  LEDBRIGHT, MUTE,
};

int i;
int persist(int argc, int *argv)
{
    // callback2 takes 2 parms.
    if (i==1) 
       callback1((int)argv[ROTATE], (int)1);

    FbWrite(" ");

    i++;
}


enum {
  MAINBTN, UPBTN, DOWNBTN, LEFTBTN, RIGHTBTN,
  HOUR, MIN, SEC,
  BADGEID, NAME, FL_ID, FL_ADDR,
  DRAWLCD8, DRBOB, ADC, ROTATE, 
  LEDBRIGHT, MUTE,
};

int i;
int persist(int argc, int *argv)
{
    // callback2 takes 2 parms.
    if (i==1) 
       callback1((int)argv[ROTATE], (int)0);
    i++;
    FbWrite(" ");
}

enum {
  MAINBTN, UPBTN, DOWNBTN, LEFTBTN, RIGHTBTN,
  HOUR, MIN, SEC,
  BADGEID, NAME, FL_ID, FL_ADDR,
  DRAWLCD8, DRBOB, ADC, ROTATE, 
  LEDBRIGHT, MUTE,
};

int i;
int persist(int argc, int *argv)
{
   char *mute;

   if (i==1) {
       callback1((int)argv[LEDBRIGHT], (char)4);

       mute = (char *)argv[MUTE];
       *mute = 1;
   }
}

