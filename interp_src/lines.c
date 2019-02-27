main()
{
   int x, y, z, a, b, c, d, e, f;
   int z1, a1, b1, c1, d1, e1, f1;
   x = 10;
   y = 10;

   setTime(22,30,0);
   FbClear();
   FbMove(10,10);
   FbWrite("testing 1 2 3");
   while (x < 90) {
      FbLine(20+x, 20+y, 10+x, 20+y);
      FbLine(10+x, 20+y, 10+x, 10+y);
      FbLine(10+x, 10+y, 20+x, 10+y);
      FbLine(20+x, 10+y, 20+x, 20+y);

      FbLine(30+x, 30+y, 10+x, 30+y);
      FbLine(10+x, 30+y, 10+x, 10+y);
      FbLine(10+x, 10+y, 30+x, 10+y);
      FbLine(30+x, 30+y, 30+x, 30+y);

      FbLine(10+x, 30+y, 20+x, 30+y);
      FbLine(20+x, 30+y, 20+x, 10+y);
      FbLine(20+x, 10+y, 10+x, 10+y);
      FbLine(10+x, 30+y, 10+x, 30+y);
      x = x + 10;
      y = y + 5;

      setNote(x,4096);
   }
   setNote(60,4096);
   FbMove(10,80);
   FbWrite(getTime());
   FbMove(10,90);
   FbWrite("abcdefghijk");

   return 12345;
}

run

