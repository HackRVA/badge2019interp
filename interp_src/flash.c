main()
{
   int w, r;

   w = flashWrite(0xabcdef01);
   printd(w);

   r=flashRead(0); printd(r);
   r=flashRead(1); printd(r);
   r=flashRead(2); printd(r);
   r=flashRead(3); printd(r);
}
run


main()
{
   int r;
   r = flashRead(0); printd(r);
   r = flashRead(1); printd(r);
   r = flashRead(2); printd(r);
   r = flashRead(3); printd(r);
}
run

main()
{
   int w;

   w=flashWrite(0xff);
   return w;
}
run

main()
{
   int r,w;

   w=flashWrite(0x01234567);
   printd(w);
   r = flashRead(0); printd(r);
}
run


main()
{
   int r,w;

   w=flashWrite(0x01234567);
   printd(w);
   r=flashRead(2);
   printd(r);
   r=flashRead(3);
   printd(r);
}
run

main()
{
   int r;

   r=flashRead(2);
   printd(r);
   r=flashRead(3);
   printd(r);
   r=flashRead(4);
   printd(r);
}
run


main()
{
   int r,w;

   w=flashWrite(0x42424242);
   printd(w);

   r=flashRead(w);
   printd(r);

   return w;
}
run


