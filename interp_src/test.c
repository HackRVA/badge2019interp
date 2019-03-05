main()
{
    setAlloc(48, 30, 5, 5, 60);
}

main()
{
    setAlloc(32, 50, 6, 6, 38);
}


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
    int d;

    d = getDPAD();
    return d;
}

int main()
{
   setAlloc(32, 38,6,6,50);
}

int main()
{
    int d;

    d = getDPAD();
    return d;
}

int main()
{
   setAlloc(32, 38,6,6,50);
}

int main()
{
   setAlloc(0, 38,6,6,50);
}

int main()
{
    int d;

    d = getDPAD();
    return d;
}

int main()
{
   setAlloc(64, 38,6,6,50);
}

int main()
{
    int b;
    b = getButton();
    return b;
}


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

int main() {
    FbMove(10, 0);
    FbWrite("hello world");
}
run

int main() {
    FbMove(10, 0);
    FbWrite("yo");
}
run


int main() {
 FbMove(10, 0);
 FbWrite("hello world");
 flareled(150, 100, 200);
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

int main() {
   int r;

   led(100, 0, 0);

   IRsend(123);
   r = IRreceive();
   FbWrite("IR done");

   led(0, 0, 0);

   exit(0);
}

int main() {
   int r;
   r = IRreceive();
   return r;
}


int main()
{
    int r,b;

    b=0;
    while (b != 4) {
	r = IRreceive();
	if (r != 0) led(r, r, r);
	b = getDPAD();
    }
   return b;
}

int main()
{
    int r;

    r=0;
    while (r == 0) {
	r = IRreceive();
	led(r, r, r);
    }
}


int main()
{
   char r;
   r = (char)IRreceive();
   led(r,r,r);

   return (int)r;
}



// bf882060 g     O *ABS*	00000000 BMXPFMSZ
main()
{
   int *x;
   x = 0xbf882060;

   return *x;
}

// bf882070 g     O *ABS*	00000000 BMXBOOTSZ
main()
{
   int *x;
   x = 0xbf882070;

   return *x;
}

// bf80f220 g     O *ABS*	00000000 DEVID
// 29D5A53

main()
{
   int *x;
   x = 0xbf80f220;

   return *x;
}

main()
{
    // 30+5+5+40 = 80% used, 20% malloc
    setAlloc(48, 30, 5, 5, 40);
}

run

new

main()
{
   int *x;
   x = malloc(64);

   return x;
}


