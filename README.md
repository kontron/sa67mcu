# SMARC-sAM67 embedded controller

This is the source code for the embedded controller on the Kontron SMARC-sAM67
board. It is powered from the standby domain and runs while the main SoC is
powered-off and provides basic house keeping services and glue logic.

## Features

- TWI host interface which exposes the control registers
- Watchdog and failsafe watchdog
- Voltage sensors
- Board settings stored in non-volatile memory
- Debug UART and debug registers

## Watchdog

The watchdog is a software reimplementation of the SMARC-sAL28 [CPLD
watchdog](https://github.com/mwalle/sl28cpld/). This way, the linux kernel
driver can be reused.

The watchdog actually consists of two watchdogs. The SMARC watchdog, which also
feed the SMARC `WDT_TIME_OUT#` signal and a failsafe watchdog. The latter is
responsible to keep the board accessible even if the main bootloader is broken.

Both watchdogs share the same counter and the same counting frequency. The
counting period is 1 second. As a register is 8 bit wide. The largest possible
timeout is 255s.

### Failsafe Mechanism

If the failsafe watchdog is not disabled by the configuration it automatically
starts when the board is powered-up or reset[^1]. The watchdog has to be stopped
by the bootloader within the timeout period of 10s. If that is not the case, the
bootsource is switched to the onboard SPI flash which contains a non-volatile
failsafe bootloader and a SoC reset is issued.

[^1]: While the failsafe watchdog is running a reset will *not* reset the
    watchdog itself. Thus, if you keep the board in reset after power is
    applied, the failsafe watchdog will bite after 10s.

## Voltage Sensors

The controller can measure (but not supervise) the board input and the RTC
voltage. The latter is tricky to measure because a measurement will always draw
a bit of power from the voltage rail. To minimize that effect, the voltage is
measured only every 10s. Also because the ADC pins of the used microcontroller
have quite a high input leakage current (compared to the nA of the RTC) it is
gated by a FET. That FET needs a voltage larger than the RTC voltage. For that,
the board features a charge pump using a simple voltage ladder. The controller
takes care of the PWM for that ladder, too.

## DEBUG UART And Debug Registers

If enabled in the configuration, a debug UART will output some startup and
runtime configuration. That UART is connected to the SoC (but shared with some
bootmode pins) and to the JTAG connector (as `SWO`). Also, if debug is enabed,
some more registers are accessible.

## Configuration

The configuration consists of at most 8 byte and is version controlled. That is,
if in the future the configuration layout will change the version field can be
increased and the controller can migrate the old configuration layout to the new
one.

### Configuration Layout v0

| Offset | Description |
| --- | --- |
| 0 | Version, must be 0 |
| 1 | Flags LSB |
| 2 | Flags MSB |
| 3 | Bootmode LSB |
| 4 | Bootmode MSB |

The following flags are supported:

| Bit | Description |
| --- | --- |
| 0 | Power-on inhibit |
| 1 | Drive bootmode pins |
| 2 | Enable watchdog by default |
| 3 | Disable failsafe watchdog by default |
| 15 | Debug enable |

## Register Map

| Offset | Name | Description |
| --- | --- | --- |
| 0 | CFG\_CTRL | Configuration control register |
| 1 | CFG\_OFS  | Configuration offset register |
| 2 | CFG\_DATA | Configuration config data register |
| 3 | VERSION | MCU version register |
| 4 | WDT\_CTRL | Watchdog control register |
| 5 | WDT\_TOUT | Watchdog timeout register |
| 6 | WDT\_KICK | Watchdog kick register |
| 7 | WDT\_CNT  | Watchdog counter register |
| 16 | BOOTMODE\_L | Bootmode (low) register |
| 17 | BOOTMODE\_H | Bootmode (high) register |
| 254 | BL\_CTRL | Bootloader control register |
| 255 | DEBUG | Debug register |

### Configuration control register

| Bit(s) | Name | Access | Description |
| --- | --- | --- | --- |
| 7 | CTRL\_BUSY | RO | Inidicates wether a configuration action is still in progress |
| 2 | CTRL\_ERASE | WO | Writing 1 to this bit triggers a configuration erase |
| 1 | CTRL\_SAVE | WO | Writing 1 to this bit triggers a configuration save |
| 0 | CTRL\_LOAD | WO | Writing 1 to this bit triggers a configuration load |

> [!NOTE]
> You must only set one bit at a time, setting multiple bits is undefined.

### Configuration offset register

| Bit(s) | Name | Access | Description |
| --- | --- | --- | --- |
| 2:0 | OFS | RW | Offset of the configuration data to access |

### Configuration config data register

| Bit(s) | Name | Access | Description |
| --- | --- | --- | --- |
| 7:0 | DATA | RW | Configuration data at the offset OFS |

### MCU version register

| Bit(s) | Name | Access | Description |
| --- | --- | --- | --- |
| 7:0 | VERSION | RO | Version of the MCU application. A value of 255 denotes an (unreleased) debug version |

### Watchdog control register

| Bit(s) | Name | Access | Description |
| --- | --- | --- | --- |
| 7 | OE1 | RW | WDT\_TIME\_OUT# output enable |
| 6 | OE0 | RW | SoC reset output enable |
| 2 | LOCK | RW | Control register lock bit. If 1, writing to the control registger is prohibited. |
| 1 | EN1 | RW | Failsafe watchdog enable |
| 0 | EN0 | RW | Watchdog enable |

> [!NOTE]
> There is just one watchdog counter. If the failsafe watchdog is enabled, the
> SoC reset output is enabled and a timeout occurs, the board will restart in
> failsafe mode.

### Watchdog timeout register

| Bit(s) | Name | Access | Description |
| --- | --- | --- | --- |
| 7:0 | TOUT | RW | Timeout in seconds |

### Watchdog kick register

| Bit(s) | Name | Access | Description |
| --- | --- | --- | --- |
| 7:0 | KICK | WO | Write the magic value 6Bh to reset the watchdog counter |

### Watchdog counter register

| Bit(s) | Name | Access | Description |
| --- | --- | --- | --- |
| 7:0 | CNT | RO | The current watchdog counter value |

### SoC bootmode (high) register

| Bit(s) | Name | Access | Description |
| --- | --- | --- | --- |
| 7:0 | BOOTMODE\_L | RW | Reading returns the low part of the current bootmode.  Writing to this register will cause the board to use that bootmode on the next reset. Returns back to its default value after the reset. |

### SoC bootmode (low) register

| Bit(s) | Name | Access | Description |
| --- | --- | --- | --- |
| 7:0 | BOOTMODE\_H | RW | Same as BOOTMODE\_L but for the high part. |

### Bootloader control register

| Bit(s) | Name | Access | Description |
| --- | --- | --- | --- |
| 7:0 | INVOKE\_BL | WO | Writing the magic value A8h to this register will cause the MCU to reset into bootloader mode. Used for updating the MCU. |

### Debug register

| Bit(s) | Name | Access | Description |
| --- | --- | --- | --- |
| 1:0 | DBG\_CMD | WO | Writing 0 will issue an ethernet PHY reset pulse. Writing 1 will issue a SoC reset. All other values are reserved. |

> [!NOTE]
> To access this register the debug configuration flag has to be set.
