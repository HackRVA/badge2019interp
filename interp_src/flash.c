/*
   change badge id (loc==1)
*/
new
main()
{
   flashErase();
   flashWrite(0x0000beef, 1);
}
run

new
main()
{
   flashErase();
}
run

/*
   change badge id (loc==1)
*/
new
main()
{
   flashWrite(0x000000AB, 1);
}
run

new
main()
{
   int r;

   r = flashRead(5); printx(r);
}
run


new
main()
{
   int r;
   r=flashRead(0); printx(r);
   r=flashRead(1); printx(r);
   r=flashRead(2); printx(r);
   r=flashRead(3); printx(r);
}
run

new
main()
{
   int r;

   flashWrite(0xabcdef01, 2);

   r=flashRead(0); printx(r);
   r=flashRead(1); printx(r);
   r=flashRead(2); printx(r);
   r=flashRead(3); printx(r);
}
run

new
main()
{
   int r;

   flashWrite(0xdeaddead, 3);

   r=flashRead(0); printx(r);
   r=flashRead(1); printx(r);
   r=flashRead(2); printx(r);
   r=flashRead(3); printx(r);
}
run


new
main()
{
   int r;

   flashWrite(0x01020304, 4);
   r = flashRead(0); printx(r);
   r = flashRead(1); printx(r);
   r = flashRead(2); printx(r);
   r = flashRead(3); printx(r);
}
run

new
main()
{
   int r;

   flashWrite(0xfedcba98, 5);

   r=flashRead(0); printx(r);
   r=flashRead(1); printx(r);
   r=flashRead(2); printx(r);
   r=flashRead(3); printx(r);
   r=flashRead(4); printx(r);
}
run

new
main()
{
   int r;

   flashWrite(0x11111111, 2);
   r = flashRead(0); printx(r);
   r = flashRead(1); printx(r);
   r = flashRead(2); printx(r);
   r = flashRead(3); printx(r);
}
run



