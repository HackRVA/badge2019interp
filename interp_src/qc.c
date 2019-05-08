void flareleds(int i)
{
    if (i < 255) flareled(0, 0, i);
    if (i < 512) flareled(0, i, 0);
    if (i < 768) flareled(i, 0, 0);
    if (i >= 768) flareled(0, 0, 0);
}

void leds(int i)
{
    if (i < 255) led(0, 0, i);
    if (i < 512) led(0, i, 0);
    if (i < 768) led(i, 0, 0);
    if (i >= 768) led(0, 0, 0);
}

void beep(int i)
{
    if (i < 64) setNote(20, 4096);
    if (i < 128) setNote(40, 4096);
    if (i < 192) setNote(60, 4096);
    if (i < 255) setNote(80, 4096);
}

int i;
int persist()
{
   i++;

   beep(i);
   leds(i);
   flareleds(i);

   if (i > 1024) i = 0;
}

