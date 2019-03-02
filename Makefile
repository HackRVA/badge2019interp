CROSS_COMPILE = /opt/microchip/xc32/v1.34/bin/xc32-

#build dir
BUILD = build

MP_PROCESSOR_OPTION = 32MX270F256D

INC += -I.
INC += -I./include
INC += -I./USB
INC += -I$(TOP)
INC += -I./$(BUILD)
INC += -D_SUPPRESS_PLIB_WARNING -D_DISABLE_OPENADC10_CONFIGPORT_WARNING -fno-schedule-insns -D_XC -D__XC
#INC += -DPAULSHACKEDBADGE

# for assembler. assembler nop's after instructions
#INC += -fno-schedule-insns 

#BADGE_CFLAGS = $(INC) -x c -c -mprocessor=32MX270F256D -Os
#BADGE_CFLAGS = $(INC) -S -c -mprocessor=32MX270F256D -O0 -g
BADGE_CFLAGS = $(INC) -c -mprocessor=32MX270F256D -O1

LIBS = /opt/microchip/xc32/v1.34/pic32mx/lib/libmchp_peripheral_32MX270F256D.a

SRC_S = 

# bootloader
LDFLAGS = -Wl,--defsym=_min_heap_size=0x100,--defsym=_min_stack_size=0x400,--gc-sections,-Map="mymap.map",--report-mem,--cref,-T,src/app_32MX270F256D.ld -Os

SRC_BADGE_C = \
	src/microchip.c src/assets.c src/assetList.c src/ir.c \
	src/timer1_int.c src/interpreter.c src/pic32config.c \
	src/buttons.c src/settings.c src/menu.c src/adc_int.c \
        src/LCDcolor.c src/S6B33.c src/badge.c src/fb.c src/tinyalloc.c

#        src/LCDcolor.c src/S6B33.c src/badge.c src/fb.c src/schedule.c

SRC_APPS_C = \
	badge_apps/adc.c badge_apps/maze.c badge_apps/xorshift.c \
	badge_apps/blinkenlights.c badge_apps/conductor.c \
	badge_apps/lasertag.c badge_apps/QC.c badge_apps/irxmit.c

#	badge_apps/QC.c badge_apps/lasertag.c

SRC_USB_C = USB/usb_device.c  USB/usb_function_cdc.c USB/usb_descriptors.c

APPOBJ = $(addprefix $(BUILD)/, $(SRC_APPS_C:.c=.o) $(SRC_APPS_S:.s=.o))

BDGOBJ = $(addprefix $(BUILD)/, $(SRC_BADGE_C:.c=.o) $(SRC_BADGE_S:.s=.o))

USBOBJ = $(addprefix $(BUILD)/, $(SRC_USB_C:.c=.o) $(SRC_USB_S:.s=.o))

# Docker stuff
TAG_COMPILER=badge-compiler:latest
TAG_IDE=badge-ide:latest
XAUTH=/tmp/.docker.xauth
DOCKER_RUN=docker run -v `pwd`:/work -w /work -u `id -u $$USER`:`id -g $$USER`
DOCKER_IDE_OPTS= --privileged -v /dev/bus/usb:/dev/bus/usb -v /tmp/.X11-unix:/tmp/.X11-unix -e HOME:/work -e DISPLAY=:0 -v ${XAUTH}:${XAUTH} -e XAUTHORITY=${XAUTH} -v `pwd`/docker/home:/home/build

all: $(BUILD)/firmware.elf $(BUILD)/firmware.hex

all-docker: docker-build-compiler
	${DOCKER_RUN} -it ${TAG_COMPILER} make clean all

ide: docker-build-ide $(XAUTH)
	${DOCKER_RUN} ${DOCKER_IDE_OPTS} ${TAG_IDE}

install: all
	sudo ./tools/bootloadit

$(APPOBJ): $(SRC_APPS_C)
	mkdir -p $(BUILD)/badge_apps
	$(CROSS_COMPILE)gcc $(BADGE_CFLAGS) -o $@ badge_apps/$(subst .o,.c,$(notdir $@))

$(BDGOBJ): $(SRC_BADGE_C)
	mkdir -p $(BUILD)/src
	$(CROSS_COMPILE)gcc $(BADGE_CFLAGS) -o $@ src/$(subst .o,.c,$(notdir $@))

$(USBOBJ): $(SRC_USB_C)
	mkdir -p $(BUILD)/USB
	$(CROSS_COMPILE)gcc $(BADGE_CFLAGS) -o $@ USB/$(subst .o,.c,$(notdir $@))


$(BUILD)/firmware.elf:	$(APPOBJ) $(BDGOBJ) $(USBOBJ)
	echo $@
	$(CROSS_COMPILE)gcc $(LDFLAGS) -o $@ $^ $(LIBS)

$(BUILD)/firmware.hex: $(BUILD)/firmware.elf
	$(CROSS_COMPILE)objdump -D $(BUILD)/firmware.elf > $(BUILD)/firmware.elf.s
	$(CROSS_COMPILE)bin2hex $(BUILD)/firmware.elf

ramsyms:	build/firmware.elf
	nm -S build/firmware.elf | grep ^a00 | sort -r -t\  -k2

#vfprintf routines are huge- 8k
romsyms:	build/firmware.elf
	nm -S build/firmware.elf | grep ^9d0 | sed -e '/ t /d' -e '/ [Aa] /d' | sort -r -t\  -k2

# more fine grain symbols filtering. see man nm
#	nm -S build/firmware.elf | sed -e '/ W /d' -e ' r /d' -e '/ D /d' -e '/ R /d' -e '/ T /d' -e '/ [Aa] /d' -e '/ N /d' -e '/ t /d' -e '/^    /d' | sort -r -t\  -k2

clean:
	find build -type f -exec rm -f \{\} \;

# DOCKER
docker-build-compiler:
	docker build --compress -t ${TAG_COMPILER} ./docker/badge-compiler/

docker-build-ide: docker-build-compiler
	docker build --compress -t ${TAG_IDE} ./docker/badge-ide/

$(XAUTH):
	xauth nlist :0 | sed -e 's/^..../ffff/' | xauth -f ${XAUTH} nmerge -

docker-kill:
	$(eval images := `docker container ls | grep "badge-" | awk '{print $$1}'`)
	if [ "$($images)" != "" ] ; then docker kill $($images); fi

docker-clean:
	docker rmi -f ${TAG_COMPILER}
	docker rmi -f ${TAG_IDE}
	docker image ls | grep none | awk '{print $$3}' | xargs docker rmi -f
