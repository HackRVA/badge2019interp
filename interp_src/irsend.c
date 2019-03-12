int i;
int persist() {

   i++;
   if ((i % 50000) != 0) return(0);

   IRsend(11, 0, (int)(i & 0xFFFF));
   setNote(75, 4096);

   print("send");
   printx((int)(i & 0xFFFF));
   IRstats();
   print("endsend");
   exit(0);
}

run

