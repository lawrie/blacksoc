CC = $(RISCV_TOOLS_PREFIX)gcc
CFLAGS = --std=gnu99 -Os -MD -Wall
OBJS = main.o icosoc.o

default: help

appimage.hex:
	echo > appimage.hex

firmware.elf: firmware.c
	$(RISCV_TOOLS_PREFIX)gcc -Os -march=rv32i -ffreestanding -nostdlib -Wall -o firmware.elf $(ICOSOC_ROOT)/common/firmware.S firmware.c \
		--std=gnu99 -Wl,-Bstatic,-T,$(ICOSOC_ROOT)/common/firmware.lds,-Map,firmware.map,--strip-debug -lgcc
	chmod -x firmware.elf

icosoc.mk: icosoc.cfg
	../../icosoc.py -f

clean::
	rm -f appimage.hex $(OBJS) *.d

include icosoc.mk
-include *.d

