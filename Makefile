CROSS_COMPILE ?= arm-none-eabi-

TAG := $(shell git describe --tags --abbrev=0 2>/dev/null || true)
TAG_COMMIT := $(shell git rev-list --abbrev=8 --abbrev-commit --tags -1)
COMMIT := $(shell git rev-parse --short=8 HEAD)

GITVER := $(shell git describe --tags --abbrev=8 2>/dev/null || true)
ifeq ($(GITVER),)
	GITVER = g$(COMMIT)
endif

ifeq ($(TAG_COMMIT),$(COMMIT))
	VERSION = $(TAG:v%=%)
else
	VERSION = 255
endif

CC = $(CROSS_COMPILE)gcc
AS = $(CROSS_COMPILE)as
LD = $(CROSS_COMPILE)gcc
OBJCOPY = $(CROSS_COMPILE)objcopy

CFLAGS  = -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb
CFLAGS += -mthumb-interwork -ffreestanding -ffunction-sections -fdata-sections
CFLAGS += -g3 -Os -gstrict-dwarf -Wall -Werror
CFLAGS += -DVERSION=$(VERSION) -DGITVER=\"$(GITVER)\"

LDFLAGS = -T linker.ld -mcpu=cortex-m0plus -mlittle-endian -g -nostdlib -mthumb -static
LIBS += -lgcc

OBJECTS  = startup.o main.o miniprintf.o uart.o systick.o adc.o vref.o misc.o
OBJECTS += iomux.o sysctl.o gpio.o cp.o i2c.o nvic.o nvm.o config.o wdt.o led.o
OBJECTS += ticks.o sl28wdt.o

DEPS := $(shell find -name '*.d')

all: mcu.elf mcu.bin

%.o: %.c
	$(COMPILE.c) -MT "$@ $(@:.o=.d)" -MMD -o $@ $<

mcu.elf: $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $@

mcu.bin: mcu.elf
	$(OBJCOPY) -O binary $< $@

.PHONY: show-version
show-version:
	@echo $(GITVER)

.PHONY: clean
clean:
	$(Q)rm -f mcu.elf mcu.bin $(DEPS) $(OBJECTS)

-include $(DEPS)
