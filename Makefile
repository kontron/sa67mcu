ifndef V
Q=@
#BRIEF+=CC AS LD AR OBJCOPY OBJDUMP RM SH LN MKDIR HOSTCC
#ECHO=printf "  %-6s %s\n" $(1) $(2)
#MSG=$@
#M=@$(call ECHO,$(TAG),$@);
#$(foreach VAR,$(BRIEF), \
#	$(eval override $(VAR) = @$$(call ECHO,$(VAR),$$(MSG)); $($(VAR))))
#$(foreach VAR,$(SILENT),$(eval override $(VAR) = @$($(VAR))))
#$(eval INSTALL = @$(call ECHO,INSTALL,$$(^:$(SRC_PATH)/%=%)); $(INSTALL))
endif

CROSS_COMPILE?=arm-none-eabi-

VERSION?=UNREL

CC = $(CROSS_COMPILE)gcc
AS = $(CROSS_COMPILE)as
LD = $(CROSS_COMPILE)gcc
OBJCOPY = $(CROSS_COMPILE)objcopy

CFLAGS += -mcpu=cortex-m0plus
CFLAGS += -mfloat-abi=soft
CFLAGS += -mlittle-endian
CFLAGS += -mthumb
CFLAGS += -mthumb-interwork
CFLAGS += -ffreestanding
CFLAGS += -ffunction-sections
CFLAGS += -fdata-sections
CFLAGS += -g3
CFLAGS += -Os
CFLAGS += -gstrict-dwarf
CFLAGS += -Wall
CFLAGS += -Werror

#CFLAGS+=-DDEBUG_UART

CFLAGS+=-DVERSION=\"$(VERSION)\"

LDFLAGS += -T linker.ld
LDFLAGS += -mcpu=cortex-m0plus
LDFLAGS += -mlittle-endian
LDFLAGS += -g
LDFLAGS += -nostdlib
LDFLAGS += -mthumb
LDFLAGS += -static
LIBS += -lgcc

OBJECTS = startup.o main.o miniprintf.o uart.o systick.o adc.o vref.o misc.o iomux.o sysctl.o gpio.o cp.o i2c.o nvic.o

DEPS := $(shell find -name '*.d')

all: mcu.elf mcu.bin

%.o: %.c
	$(COMPILE.c) -MT "$@ $(@:.o=.d)" -MMD -o $@ $<

mcu.elf: $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $@

mcu.bin: mcu.elf
	$(OBJCOPY) -O binary $< $@

.PHONY: clean
clean:
	$(Q)rm -f mcu.elf mcu.bin $(DEPS) $(OBJECTS)

-include $(DEPS)
