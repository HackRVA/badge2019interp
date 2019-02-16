# badge2019interp
badge2019 hardware with the c interpreter working

***NOTE***
no microchip project file yet. use "make"


***Interpreter***

Paste one of the "main" examples from led.c into
a screen/terminal session and then type "run"

also see tools/sendbadge.py


*example*

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


***Code overview***

  microchip.c       - init chip, setup USB framework
    badge.c         - main code
      processIO     - "main"
        buttons.c     - service button presses
        IRhandler.c   - service IR packets, do callbacks
        menu.c        - use button press to navigate menus
           badge_apps/*  - runningApp() is set to one of these by menu.c

      UserInit        - "user" hardware init
			bootup pin directions and pullups- has to run fast and first

			LCD init      - initial pin, high level LCD functions
			S6B33.c       - low level LCD contoller code
			fb.c          - framebuffer init, opengl-ish draw functions
			timer1_int.c  - audio timer, low level IR, PWM/LED timer
			adc_int.c     - analog->digital hw setup and interupt init
			ir.c          - high level IR service callbacks
			settings.c    - menus for various prefs and settings
			assets.c      - asset playback functions, audio, image display
			assetsListc   - generic access of assets via enums ids



***Programming overview***

   Makefile   
	make
		makes build/firmware.hex   - used to program badge over USB with bootloadit
		makes build/firmware.hex.s - assembler dump of hex for folks who can read asm

	make ramsyms
		dumps ram variable usage. If you run out of ram this is a sorted list of usage
                                          assets are the usual suspect, or arrays
                                          most assets should be "const" which leaves them in flash 
                                               instead of copying to ram
	make romsyms
		dumps rom variable usage. If you run out of rom this is a sorted list of usage
                                          assets are the usual problem, or arrays
                                          any images are suspect since they are typically large

        tools/bootloadit
                program badge over USB using bootloader
                tools/bootloadit  will upload build/firmware.hex to the badge

        tools/progboot.sh
                program bootloader using pickit3 and hex from tools/bootloader_2019.hex

        tools/prog.sh
                program build/firmware.hex using pickit3. You probably don't want this

        tools/sendbadge.py
                send standard input to badge c interpreter via USB using /dev/ttyACM0
                cat file.c | python tools/sendbadge.py


** interpreter stats **

    R XXXXXX  - return or exit() code from interpreter main()
    T XXXXXX  - program size
    D XXXXXX  - program data size
    S XXXXXX  - stack size
    Y XXXXXX  - symbol table

current ram allocation:

   #define TEXTSECTION 128 // * 4 
   #define DATASECTION 128 // * 1 
   #define STACKSECTION 128 // * 4 
   #define SYMBOLSECTION 512 // * 4 

