int main() {
   int r, b, ir;

   while (1)  {
      r = IRreceive();
      if (r == 0) {
	setNote(40, 4096);
        led(255, 0, 0);
      }
      else {
	setNote(100, 4096);
        led(0, 255, 0);
      }

      print("recv");
      IRstats();
      print("endrecv");

      b = 0;
      while (b==0)  {
         b = getButton();
      }
   }

   exit(r);
}

run

