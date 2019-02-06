// Configuring the Device Configuration Registers
#pragma config UPLLEN   = ON        // USB PLL Enabled
#pragma config UPLLIDIV = DIV_1     // USB PLL Input Divider = Divide by 1 (board has 4 mhz crystal)

#pragma config DEBUG    = OFF           // Background Debugger disabled
#pragma config FPLLMUL = MUL_20         // PLL Multiplier: Multiply by 20 ( * 4mhz)
#pragma config FPLLIDIV = DIV_2         // PLL Input Divider:  Divide by (20 * 4mhz) / 2 = 40mhz

#pragma config FVBUSONIO = OFF		// PEB 20170409 need this for up button
#pragma config OSCIOFNC = OFF           // PEB 20180306 CLKO Output Signal Active on the OSCO Pin (Disabled)
#pragma config FSOSCEN = OFF            // Secondary Oscillator Enable (Disabled)

#pragma config FPLLODIV = DIV_1         // PLL Output Divider: Divide by 1: 40mhz / 1 = 40mhz

#pragma config FWDTEN = OFF             // WD timer: OFF
#pragma config POSCMOD = XT             // Primary Oscillator Mode: High Speed xtal
#pragma config FNOSC = PRIPLL           // Oscillator Selection: Primary oscillator  w/ PLL
#pragma config FPBDIV = DIV_1           // Peripheral Bus Clock: Divide by 1
#pragma config BWP = OFF                // Boot write protect: OFF

#pragma config ICESEL = ICS_PGx2    // ICE pins configured on PGx1 (PGx2 is multiplexed with USB D+ and D- pins).
