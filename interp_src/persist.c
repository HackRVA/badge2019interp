//
// Badge usb commands
//
// new - clear source ram
// run - run source ram
// list - list source ram
// reset - reinitialize interpreter, reset allocation
// lib - list bindings and parameters
// stats - print ram stats
//
// Bindings are passed as argc, argv into
//    main(argc, argv)  - runs just once
//    
//    persist(argc, argv) - run once per badge loop
//
// To stop persist() do:
//   new <enter>
//   run <enter>
// this emptys the src buffer and clears interpreter persist()
//
// If code gets too big use setAlloc():
//   setAlloc(int allocScanlines, int textpct, int datapct, int stackpct, int symbolpct) 
// 
// if allocScanlines == 0 then use interpreterRam which is ~8k
// allocated scanlines are at the bottom of the display                                        
// so limit print/drawing between (top, bottom-alloc)
// and use pushbuffers() to update since it doesn't clear the display ram
//
// if the sum of percentages is not 100% then the remaining is used for malloc() 
// alloc changes persist until the next alloc() or "reset"
//
// new
// main()
// {
//     setAlloc(0, 30, 5, 5, 40); // 20% free from interpreter ram
// }
// run

// new
// main()
// {
//     setAlloc(64, 30, 5, 5, 40); // steal 64 scan from display for 16k of ram -> (64 * 128 * 2) with 20% free for malloc (100-(30+5+5+40))
// }
// run


//
// badge functions args types and these have to be the same
// because of int <-> char alignment otherwise will fault and lockup badge
// btns are unsigned char types but interp doesn't have "unsigned"

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

   // setup pointers from binding array
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


// variable access
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



// rotate display
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
    // callback1 takes 1 parms.
    // just run once to set variable
    if (i==1) {
       callback1((int)argv[ROTATE], (int)1); // 1==rotated
       //callback1((int)argv[ROTATE], (int)0); // 0==normal
       FbWrite(""); 
    }
    i++;
}

// 
// call an app
// may not work with all of them
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
    // callback0 takes no parms.
    // adc runs every time
    callback0((int)argv[ADC]);
}

//
// on/off buzzer
//
enum {
  MAINBTN, UPBTN, DOWNBTN, LEFTBTN, RIGHTBTN,
  HOUR, MIN, SEC,
  BADGEID, NAME, FL_ID, FL_ADDR,
  DRAWLCD8, DRBOB, ADC, ROTATE, 
  LEDBRIGHT, MUTE,
};


int main(argc, int *argv)
{
   char *mute;
   int x;

   mute = (char *)argv[MUTE];
   printx((int)mute);
   // *mute = 1; // mute
   *mute = 0;
}


// click on whatever menu is under cursor
enum {
  MAINBTN, UPBTN, DOWNBTN, LEFTBTN, RIGHTBTN,
  HOUR, MIN, SEC,
  BADGEID, NAME, FL_ID, FL_ADDR,
  DRAWLCD8, DRBOB, ADC, ROTATE, 
  LEDBRIGHT, MUTE,
};

int main(argc, int *argv)
{
   char *mainbtn;

   mainbtn = (char *)argv[MAINBTN];
   *mainbtn = 200; // clicks on things

   return (int)*mainbtn;
}


// malloc doesnt work with standard alloc
// since 100% of ram is allocated
main()
{
    setAlloc(0, 30, 5, 5, 40); // 20% free
}
run
new

// test passing int and returning int
enum {
  MAINBTN, UPBTN, DOWNBTN, LEFTBTN, RIGHTBTN,
  HOUR, MIN, SEC,
  BADGEID, NAME, FL_ID, FL_ADDR,
  DRAWLCD8, DRBOB, ADC, ROTATE, 
  LEDBRIGHT, MUTE, RETVAL,
};

int main(argc, int *argv)
{
   int r, i;
   int *arr, *a;

   arr = malloc(64*4); // in bytes
   a = arr;

   printx(a);
   i=0;
   while (i < 64) {
      *a++ = callback1((int)argv[RETVAL], (int)i);
      i++;
   }

   a = arr;
   i=0;
   while (i < 64) {
      printd(*a++);
      i++;
   }

   return 42;
}


