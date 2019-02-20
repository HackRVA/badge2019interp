int main() {
   int r, b, ir;

   while (1)  {
      IRsend(31);
      setNote(75, 4096);

      print("send");
      IRstats();
      print("endsend");

      b = 0;
      while (b==0)  {
         b = getButton();
      }
   }

   exit(b);
}

run

