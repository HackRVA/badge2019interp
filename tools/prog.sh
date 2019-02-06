#!/bin/tcsh -f

set mplab = "/opt/microchip/mplabx/v3.61/mplab_ide/bin/"

cat << END > /tmp/prog.x
device pic32MX270F256D
set poweroptions.powerenable true
set voltagevalue 3.3
hwtool pickit3 -p
program "./build/firmware.hex" 
quit
END

$mplab/mdb.sh /tmp/prog.x

