# badge2019interp
badge2019 hardware with the c interpreter woking

paste one of the "main" examples from led.c into
a screen/terminal session and then type "run"
EG:

start screen:
screen /dev/ttyACM0

You may have to paste this in 2 pieces because
of a USB overrun error. You will know if it overruns
if the paste result doesn't look like below.

int main() {
   int r;

   led(100, 0, 0);

   IRsend(123);
   IRreceive(&r);
   FbWrite("IR done");

   led(0, 0, 0);
}

run

