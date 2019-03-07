# badge2019interp
badge2019 hardware with the c interpreter working

***NOTE***
no microchip project file yet. use "make"



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


**Interpreter**

It is no where near a full c implementation:

- Arrays have to use malloc()
- looping uses while
- no case

*Keywords*

```
char else enum if int return sizeof while
print printd printx malloc memset memcmp
flareled led FbMove FbWrite
IRreceive IRsend setNote getButton getDPAD
IRstats setTime getTime FbLine FbClear
flashWrite flashRead setAlloc
exit void main
```


*Control commands*
```
run   - run source buffer
new   - clear source buffer
list  - list source buffer
reset - reset memory allocation to safe numbers
```

*Execution*/

Note:
The source buffer is 2K which may need need to be expanded

Paste one of the "main" blocks from 
```
interp\_src/\*.c

```
files into a screen/terminal session and then type "run"

you can also just setup to read serial out and cat:
```
cat /dev/ttyACMO &
cat test.c > /dev/ttyACM0
```

a file can have multiple 'main' if the interpreter
is cleared between each 'main':

```
main()
{
    // setAlloc below changes memory allocation
    // from the default ~8k internal ram

    // note: don't write into the 32 scanlines
    // that are allocated from the LCD buffer
    // at the bottom of the display:

    //   0 - 103 display
    // 104 - 132 interpreter ram

    // parameters:
    //  32 scanlines
    // 50% text area (program)
    //  6% Data (variables)
    //  6% stack
    // 38% symbol table

    // unused % of ram will be used
    // for the interpreter "malloc" routine
    setAlloc(32, 50, 6, 6, 38);

    // default
    // setAlloc(0, 38, 6, 6, 50);
}

run

new

int main() {
 FbMove(10, 0);
 FbWrite("hello world");
 flareled(150, 100, 200);
 led(0, 0, 100);
 return 123;
}

run
```

If you get this error:

*duplicate global declaration*

it usually means you forgot to do a "new"
sometimes it means the stack overwrote
the symbol table



**Interpreter run results**

```
    R XXXXXX  - return or exit() code from interpreter main()
    T XXXXXX  - program size
    D XXXXXX  - program data size
    S XXXXXX  - stack size
    Y XXXXXX  - symbol table
```

** BUGS **

 - yeah
