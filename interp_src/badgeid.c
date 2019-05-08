enum {
  MAINBTN, UPBTN, DOWNBTN, LEFTBTN, RIGHTBTN,
  HOUR, MIN, SEC,
  BADGEID, NAME, FL_ADDR,
  DRAWLCD8, DRBOB, ADC, ROTATE, 
  LEDBRIGHT, MUTE,
};

int i;
int persist(int argc, int *argv)
{
   int *fbadgeid;

   fbadgeid = (char *)argv[BADGEID];

   if (i == 0) printx(fbadgeid);
   if (i == 1) printx(*fbadgeid);
   i = i + 1;

   return 0;
}
