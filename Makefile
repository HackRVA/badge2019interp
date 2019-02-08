BUILD = build

CROSS_COMPILE = /opt/microchip/xc32/v1.34/bin/xc32-

MP_PROCESSOR_OPTION = 32MX270F256D

INC += -I.
INC += -I./include
INC += -I./USB
INC += -I$(TOP)
INC += -I./$(BUILD)
INC += -D_SUPPRESS_PLIB_WARNING -D_DISABLE_OPENADC10_CONFIGPORT_WARNING -fno-schedule-insns -D_XC -D__XC -mno-float
# assembler nop's after instructions
#INC += -fno-schedule-insns 

# for plib
BADGE_CFLAGS = $(INC) -x c -c -mprocessor=32MX270F256D -Os

LIBS = /opt/microchip/xc32/v1.34/pic32mx/lib/libmchp_peripheral_32MX270F256D.a
#LIBS = /opt/microchip/xc32/v1.34/pic32mx/lib/no-float/libmchp_peripheral_32MX270F256D.a

SRC_S = 

# bootloader
LDFLAGS = -Wl,--defsym=_min_heap_size=0x400,--defsym=_min_stack_size=0x100,--gc-sections,-Map="mymap.map",--report-mem,--cref,-T,src/app_32MX270F256D.ld -mno-float -Os

SRC_BADGE_C = src/LCDcolor.c src/S6B33.c src/badge.c src/fb.c \
	src/microchip.c src/assets.c src/assetList.c src/ir.c \
	src/timer1_int.c src/interpreter.c src/pic32config.c \
	src/adc_int.c src/buttons.c src/settings.c src/menu.c

SRC_USB_C = USB/usb_device.c  USB/usb_function_cdc.c USB/usb_descriptors.c

BDGOBJ = $(addprefix $(BUILD)/, $(SRC_BADGE_C:.c=.o) $(SRC_BADGE_S:.s=.o))

USBOBJ = $(addprefix $(BUILD)/, $(SRC_USB_C:.c=.o) $(SRC_USB_S:.s=.o))

all: $(BUILD)/firmware.elf $(BUILD)/firmware.hex

$(BDGOBJ): $(SRC_BADGE_C)
	mkdir -p $(BUILD)/src
	$(CROSS_COMPILE)gcc $(BADGE_CFLAGS) -o $@ src/$(subst .o,.c,$(notdir $@))

$(USBOBJ): $(SRC_USB_C)
	mkdir -p $(BUILD)/USB
	$(CROSS_COMPILE)gcc $(BADGE_CFLAGS) -o $@ USB/$(subst .o,.c,$(notdir $@))

$(BUILD)/firmware.elf:  $(OBJ) $(BDGOBJ) $(USBOBJ) 
	echo $@
	$(CROSS_COMPILE)gcc $(LDFLAGS) -o $@ $^ $(LIBS)

$(BUILD)/firmware.hex: $(BUILD)/firmware.elf
	$(CROSS_COMPILE)objdump -D $(BUILD)/firmware.elf > $(BUILD)/firmware.elf.s
	/opt/microchip/xc32/v1.34/bin/xc32-bin2hex $(BUILD)/firmware.elf
