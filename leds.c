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

int main()
{
    setNote(100, 4096);
}



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



int main()
{
    int d;
    d = getDPAD();
    return d;
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
    FbWrite("hello world");
}

int main() {
    FbMove(50, 40);
    FbWrite("yo");
}


int main() {
 FbWrite("hello world");
 flareled(100, 10, 200);
 led(0, 0, 100);
 return 123;
}

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
    while (b == 0) {
	r = IRreceive();
	if (r != 0) led(r, r, r);
	b = getButton();
    }
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


