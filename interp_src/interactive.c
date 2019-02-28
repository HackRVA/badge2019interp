int main()
{
    int d;
    d = getDPAD();
    return d;
}
run

int main()
{
    int b;
    b = getButton();
    return b;
}
run


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
run

// bf882060 g     O *ABS*	00000000 BMXPFMSZ
main()
{
   int *x;
   x = 0xbf882060;

   return *x;
}
run

// bf882070 g     O *ABS*	00000000 BMXBOOTSZ
main()
{
   int *x;
   x = 0xbf882070;

   return *x;
}
run

// bf80f220 g     O *ABS*	00000000 DEVID
// 29D5A53

main()
{
   int *x;
   x = 0xbf80f220;

   return *x;
}


