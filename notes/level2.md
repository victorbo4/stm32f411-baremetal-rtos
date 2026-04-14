# Level 2 - GPIO (General Purpose I/O)

### Main goal

The objective in this level is gonna be to blink an internal LED (LD3).
This is connected to the I/O PD13 of the STM32.

> Reference: User Manual 1842, pág. 16. 

To achieve this, we will have to:
- Complete `board_config.h` with the corresponding GPIOD structure (ref. rm0383, gpio section).
- Define in `board_config.h` the necessary bit masks.
- Start the AHB1 clock bus (I/O D port or something like that). (ref. rm0383, rcc section).
- Configure the pin in a new file, `gpio.h/c`.
- Create a function to make the LED blink in `main.c`.


