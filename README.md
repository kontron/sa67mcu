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
