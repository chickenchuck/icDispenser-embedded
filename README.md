# icDispenser-embedded
IC (Integrated Circuit) Dispenser AVR Program, C language
Written by Andrew Hollabaugh, started 1/14/19, major rewrite 6/16/19, still in development.

IC Dispenser: A machine that stores and indexes many IC tubes, with the ability to dispense individual ICs from selected tubes.

For use with ATmega168/328

No libraries are used

[Platformio](https://github.com/platformio/platformio-core) is used for development.

Abbreviations:
sel = tube selector
dis = dispenser

### Files
- main.h, main.c
- steppers.h, steppers.c - low-level functionality for moving steppers
- usart.h, usart.c - setup of USART peripheral, serial command parsing
- sel.h, sel.c - tube selector functionality
- dis.h, dis.c - dispensing functionality
- queue.h, queue.c - simple queue, used as a serial rx buffer

### IO Devices
- Serial communication with master device (probably Raspberry Pi)
- Stepper driver for selector motor
- Stepper driver for dispenser motor
- IR sensor (beam/break) for index positioning
- IR sensor (beam/break) for detecting when ICs are dispensed
- Limit switch for dispenser homing

### Processor peripherals used
- TIMER0: for timing selector motor pulses
- TIMER2: for timing dispenser motor pulses
- INT0 (interrupt 0): connected to dispenser limit switch for knowing when to stop homing dispenser
- PCINT10 (pin change interrupt 10): connected to selector limit switch for keeping track of its position
- USART0: serial communications with master device

### Serial commands
#### Commands without arguments
These commands are just a single, uppercase character with a newline
- E: enables selector motor (hold position)
- D: disables selector motor (moves freely)
- H: homes the selector motor (goes to index 0)
- O: moves to the next index (only used for debug)
- N: enables dispenser motor (hold position)
- S: disables dispenser motor (moves freely)
- R: moves the dispenser backwards until it hits the limit switch

#### Commands with arguments
These commands are an uppercase character followed by a number of up to five digits, terminated by a newline
- M: sets the selector to move to a desired index
- T: sets the total number of tubes in the dispenser
- I: sets the dispenser to dispense a specified number of ICs and home afterwards
- Q: same as I, but does not home afterwards

