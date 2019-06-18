# icDispenser-embedded
IC (Integrated Circuit) Dispenser AVR Program (C)
Written by Andrew Hollabaugh, started 1/14/19, rewritten 6/16/19

IC Dispenser: A machine that stores and indexes many IC tubes, with the ability to dispense individual ICs from selected tubes.

For use with ATmega168/328

No libraries are used

All functionality is run through interrupts; the main loop does nothing. 

[Platformio](https://github.com/platformio/platformio-core) is used for development.

Some abbreviations used:
sel = tube selector
dis = dispenser

### Files
- main.h, main.c
- steppers.h, steppers.c - low-level functionality for moving steppers
- USART.h, USART.c - setup of USART peripheral, serial command parsing
- accel.h, accel.c - communication with MPU6050 accelerometer
- sel.h, sel.c - tube selector functionality
- dis.h, dis.c - dispensing functionality

### IO Devices
- Serial communication with master device (probably Raspberry Pi)
- Stepper driver for selector motor
- Stepper driver for dispenser motor
- IR sensor (beam/break) for index positioning
- Limit switch for dispenser homing
- MPU6050 accelerometer for sensing when ICs fall in container

### Processor peripherals used
- TIMER0: for timing selector motor pulses
- TIMER2: for timing dispenser motor pulses
- INT0 (interrupt 0): connected to dispenser limit switch for knowing when to stop homing dispenser
- INT1 (interrupt 1): connected to selector limit switch for keeping track of its position
- USART0: serial communications with master device
- TWI (I2C): for communications with MPU6050 accelerometer

### Serial commands
- E: enables selector motor (hold position)
- D: disables selector motor (moves freely)
- H: homes the selector motor so it is at index 0
- M: sets the selector to move to a desired index
- O: moves to the next index, probably just for debug
- T: sets the total number of tubes in the dispenser
- N: enables dispenser motor (hold position)
- S: disables dispenser motor (moves freely)
- I: sets the dispenser to dispense a specified number of ICs and home afterwards
- R: moves the dispenser backwards until it hits the limit switch
- Q: same as I, but does not home afterwards

