int main()
{
    int i;

    i=0;
    while (i <= 255) {
	led(0, 0, i);
        i = i + 1;
    }

    led(128, 128, 128);
}
run

int main()
{
    setNote(100, 4096);
}
run



void beep()
{
    int i;
    i=0;
    while (i <= 255) {
	setNote(i, 4096);
        i = i + 5;
    }
}

int main()
{
    int i;

    i=0;
    while (i <= 255) {
	beep();
        i = i + 5;
    }
}
run






int main()
{
    int i;

    i=0;
    while (i <= 255) {
	led(0, i, i);
        i = i + 1;
    }

    i=0;
    while (i <= 255) {
	led(i, 0, 0);
        i = i + 1;
    }
}
run


int main() {
    FbWrite("hello world");
}
run

int main() {
    FbMove(50, 40);
    FbWrite("yo");
}
run


int main() {
 FbWrite("hello world");
 flareled(100, 10, 200);
 led(0, 0, 100);
 return 123;
}
run


int main(){
 int r;
 IRsend(123);
 r = IRreceive();
 FbWrite("IR done");
 led(100, 0, 0);
 exit(r);
}
run

int main() {
   int r;

   led(100, 0, 0);

   IRsend(123);
   r = IRreceive();
   FbWrite("IR done");

   led(0, 0, 0);

   exit(0);
}
run

int main() {
   int r;
   r = IRreceive();
   return r;
}
run



int main()
{
   char r;
   r = (char)IRreceive();
   led(r,r,r);

   return (int)r;
}
run

