/*
   change badge id (loc==1)
*/
new
main()
{
   flashWrite(0x0000beef, 1);
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
   flashWrite(41, 1);
}
run

new
main()
{
   int r;

   r = flashRead(5); printx(r);
}
run
