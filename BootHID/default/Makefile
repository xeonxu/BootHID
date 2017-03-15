###############################################################################
# Makefile for the project BootHID
###############################################################################

## General Flags
PROJECT			?= BootHID
MCU			?= atmega32u4
TARGET			?= BootHID.elf
F_CPU			?= 16000000UL
DEV_PORT		?= /dev/tty.usbmodem14141
BOOTLOADER_ADDRESS	?= 0x7c00
CC                       = avr-gcc
CPP                      = avr-g++


## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -gdwarf-2 -DF_CPU=$(F_CPU) -Os -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d 

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)

ifeq ($(MCU), atmega32u4)
LDFLAGS += -T ../avr5b.x -Wl,-Map=BootHID.map
endif
ifeq ($(MCU), atmega16u4)
LDFLAGS += -T ../avr5b.x -Wl,-Map=BootHID.map
endif
ifeq ($(MCU), at90usb646)
LDFLAGS += -T ../avr5b.x -Wl,-Map=BootHID.map
endif
ifeq ($(MCU), at90usb1286)
LDFLAGS += -T ../avr51b.x -Wl,-Map=BootHID.map
endif
ifeq ($(MCU), atmega8u2)
LDFLAGS += -T ../avr35b.x -Wl,-Map=BootHID.map
endif
ifeq ($(MCU), atmega16u2)
LDFLAGS += -T ../avr35b.x -Wl,-Map=BootHID.map
endif
ifeq ($(MCU), atmega32u2)
LDFLAGS += -T ../avr35b.x -Wl,-Map=BootHID.map
endif
ifeq ($(MCU), at90usb82)
LDFLAGS += -T ../avr35b.x -Wl,-Map=BootHID.map
endif
ifeq ($(MCU), at90usb162)
LDFLAGS += -T ../avr35b.x -Wl,-Map=BootHID.map
endif
LDFLAGS += -Wl,-section-start=.text=$(BOOTLOADER_ADDRESS)


## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom -R .fuse -R .lock -R .signature

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings


## Include Directories
INCLUDES = -I"../includes" 

## Objects that must be built in order to link
OBJECTS = usb_hid.o main.o vt.o 

## Objects explicitly added by the user
LINKONLYOBJECTS = 

## Build
all: $(TARGET) BootHID.hex BootHID.eep BootHID.lss ## Compile

usb_hid.o: ../usb_hid.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

main.o: ../main.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

vt.o: ../vt.S
	$(CC) $(INCLUDES) $(ASMFLAGS) -c  $<

##Link
$(TARGET): $(OBJECTS)
	 $(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

%.hex: $(TARGET)
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: $(TARGET)
	-avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0

%.lss: $(TARGET)
	avr-objdump -h -S $< > $@

program: $(TARGET:.elf=.hex)
	avrdude -P $(DEV_PORT) -b 19200 -c avrisp -p m32u4 -v -e -U lfuse:w:0xFF:m -U hfuse:w:0xDD:m -U efuse:w:0xF3:m
	avrdude -P $(DEV_PORT) -b 19200 -c avrisp -p m32u4 -v -e -U flash:w:$< -U lock:w:0x2F:m


# Special rules for generating hex files for various devices and clock speeds

ALLHEXFILES = hexfiles/at90usb82_8mhz.hex hexfiles/at90usb162_8mhz.hex hexfiles/at90usb646_8mhz.hex \
hexfiles/atmega8u2_8mhz.hex hexfiles/atmega16u2_8mhz.hex hexfiles/atmega32u2_8mhz.hex hexfiles/atmega16u4_8mhz.hex \
hexfiles/atmega32u4_8mhz.hex hexfiles/at90usb82_16mhz.hex hexfiles/at90usb162_16mhz.hex hexfiles/at90usb646_16mhz.hex \
hexfiles/atmega8u2_16mhz.hex hexfiles/atmega16u2_16mhz.hex hexfiles/atmega32u2_16mhz.hex \
hexfiles/atmega16u4_16mhz.hex hexfiles/atmega32u4_16mhz.hex  #hexfiles/at90usb1286_16mhz.hex hexfiles/at90usb1286_8mhz.hex

allhexfiles: $(ALLHEXFILES)
	$(MAKE) clean
	avr-size hexfiles/*.hex

$(ALLHEXFILES):
	@[ -d hexfiles ] || mkdir hexfiles
	@device=`echo $@ | sed -e 's|.*/||g' -e 's|_.*||g'`; \
	clock=`echo $@ | sed -e 's|.*_||g' -e 's|mhz.*||g'`; \
	echo "$$device"|grep "usb" > /dev/null; \
	if [ $$? = 0 ];then \
	addr=`echo $$device | sed -e 's/at90usb\([0-9]\+\)[0-9]\{1\}/\1/g' | awk '{printf("%x", ($$1 - 1) * 1024)}'`; \
	else \
	addr=`echo $$device | sed -e 's/atmega\([0-9]\+\)u.*/\1/g' | awk '{printf("%x", ($$1 - 1) * 1024)}'`; \
	fi; \
	echo "### Make with F_CPU=$${clock}000000 DEVICE=$$device BOOTLOADER_ADDRESS=$$addr"; \
	$(MAKE) clean; \
	$(MAKE) main.hex F_CPU=$${clock}000000 MCU=$$device BOOTLOADER_ADDRESS=$$addr
	mv main.hex $@


## Other dependencies
-include $(shell mkdir -p dep 2>/dev/null) $(wildcard dep/*)

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) BootHID.elf dep BootHID.hex BootHID.eep BootHID.lss BootHID.map *~

