int i;
int persist() {
   int r, b, ir;

   led(0, 0, 0);
   i++;
   if ((i % 50000) != 0) return(0);

   r = IRreceive();
   if (r == 0x80000000) {
	setNote(40, 4096);
        led(255, 0, 0);
   }
   else {
	setNote(100, 4096);
        led(0, 255, 0);
   }
   printx(r);
   print("recv");
   IRstats();
   print("endrecv");

   exit(r);
}

run

