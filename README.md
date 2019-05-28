# icDispenser-embedded
IC (Integrated Circuit) Dispenser AVR Program (C)
Written by Andrew Hollabaugh, started 1/14/19

IC Dispenser: A machine that stores and indexes many IC tubes, with the ability to dispense individual ICs from selected tubes.

For use with ATmega168/328

No libraries are used

All functionality is run through interrupts; the main loop does nothing. 

[Platformio](https://github.com/platformio/platformio-core) is used for development.

###IO Devices
- Serial communication with master device (probably Raspberry Pi)
- Stepper driver for selector motor
- Stepper driver for dispenser motor
- IR sensor (beam/break) for index positioning
- Limit switch for dispenser homing

###Processor peripherals used
- TIMER0: for timing selector motor pulses
- TIMER2: for timing dispenser motor pulses
- INT0 (interrupt 0): connected to dispenser limit switch for knowing when to stop homing dispenser
- INT1 (interrupt 1): connected to selector limit switch for keeping track of its position
- USART0 - serial communications with master device

###Serial commands
- ENABLE: enables selector motor (hold position)
- DISABLE: disables selector motor (moves freely)
- HOME: homes the selector motor so it is at index 0
- MOVE_TO_POS: sets the selector to move to a desired index
- MOVE_ONE: moves to the next index, probably just for debug
- NR_ITEMS: sets the total items in the dispenser
- ENABLE_DISPENSER: enables dispenser motor (hold position)
- DISABLE_DISPENSER: disables dispenser motor (moves freely)
- DISPENSE: sets the dispenser to move forwards an amount of millimeters and home afterwards
- HOME_DISPENSER: moves the dispenser backwards until it hits the limit switch
- TEST_DISPENSER: same as DISPENSE, but does not home afterwards

