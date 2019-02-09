int main()
{
    int i;

    i=0;
    while (i <= 255) {
	led(0, 0, i);
        i = i + 1;
    }

    i=0;
    while (i <= 10000) {
        i = i + 1;
    }

    i=0;
    while (i <= 255) {
	led(i, 0, 0);
        i = i + 1;
    }

    i=0;
    while (i <= 10000) {
        i = i + 1;
    }

    i=0;
    while (i <= 255) {
	led(0, i, 0);
        i = i + 1;
    }

    i=0;
    while (i <= 10000) {
        i = i + 1;
    }

    led(128, 128, 128);
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
    FbMove(0, 40);
    FbWrite("yo");
}

int main() {
   led(100, 0, 0);
}

int main() {
   int r;

   led(100, 0, 0);

   IRsend(123);
   IRreceive(&r);
   FbWrite("IR done");

   led(0, 0, 0);
}

int main() {
   int r;
   IRreceive(&r);
   return r;
}


