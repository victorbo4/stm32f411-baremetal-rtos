# STM32 Real-Time Data Acquisition System
### Bare-metal → RTOS firmware on STM32F411 (ARM Cortex-M4)

> No HAL. No CubeMX. Every peripheral configured by hand from RM0383.  
> Drivers written in C, debugged with OpenOCD + GDB on real hardware.

---

## What this is

This is a learning project, built progressively from bare startup code up to a full FreeRTOS pipeline. Each stage is finished and verified before the next one starts.

I built this to bridge the gap between theory and practice. While university courses provide the fundamentals of microcontrollers and tools like STM32CubeMX, there is often a "missing link" between high-level configuration and the actual hardware behavior. This project is my way of connecting those dots by implementing the system at the register level.

The system acquires analog signals via ADC, processes them in separate RTOS tasks, and sends results over UART — without the CPU ever blocking or polling.

**Target pipeline:**
```
Timer Trigger → ADC + DMA → FreeRTOS Queue → Processing Task → UART DMA → PC
```

**Main constraints I imposed on myself:**
- No HAL, no CubeMX — register level only, RM0383 open next to the editor
- MISRA-C mandatory rules as a discipline
- Every module testable and inspectable via GDB before moving to the next

---

## Toolchain

| Category | Tool |
|----------|------|
| Language | C (C99) |
| Hardware abstraction | CMSIS (reg. definitions only) |
| Compiler | arm-none-eabi-gcc |
| Build | Make |
| Debug | OpenOCD + GDB |
| Static analysis | cppcheck + MISRA addon |
| Code formatting | clang-format |
| Serial com | tio |
| OS | Linux (Arch) |
| Editor | Neovim |

## Platform

| Item | Detail |
|------|--------|
| MCU | STM32F411VE (ARM Cortex-M4 @ 96 MHz) |
| Board | STM32F411E-DISCO (Discovery) |
| RTOS | FreeRTOS |

---

## Repository structure

```
/core
    startup.c           # Vector table, .data/.bss init, stack setup
    system_clock.c      # PLL configuration, RCC registers

/drivers
    gpio/               # MODER, OTYPER, OSPEEDR, PUPDR, ODR, BSRR
    uart/               # Baudrate calc, TX/RX, non-blocking with ring buffer
    adc/                # Continuous mode, DMA circular buffer
    dma/                # Stream/channel config, transfer complete IRQ
    timer/              # TIM2 periodic interrupt at 1 kHz

/rtos
    freertos_port/      # FreeRTOS kernel + port for Cortex-M4

/app
    acquisition_task.c  # Reads ADC DMA buffer, pushes to queue
    processing_task.c   # Consumes queue, computes metrics
    communication_task.c# Serializes results, sends over UART

/platform
    board_config.h      # Pin mapping, clock constants, buffer sizes
```

---

## Development roadmap

Each level is independently testable. I don't move forward until GDB confirms the current stage is working correctly.

### ✅ Level 0 — Startup and linker
- Custom linker script (`memory.ld`)
- Manual vector table in C
- `.data` copy from flash, `.bss` zero-fill
- Stack pointer setup
- **Goal:** `main()` runs with zero runtime dependencies

### 🔄  Level 1 — Clock system
- HSI → PLL → SYSCLK at 96 MHz
- PLLM / PLLN / PLLP / PLLQ calculated by hand from the clock tree
- AHB / APB prescalers configured
- Verified via GDB: `RCC_CR`, `RCC_PLLCFGR`, `RCC_CFGR`

### ⬜  Level 2 — GPIO driver
- `gpio_init()`, `gpio_set()`, `gpio_toggle()`
- Direct control of MODER, OTYPER, OSPEEDR, PUPDR, ODR, BSRR
- **Goal:** LED blink with zero HAL

### ⬜  Level 3 — Timer and interrupts
- TIM2 at 1 kHz, NVIC configuration
- No busy-wait anywhere

### ⬜ Level 4 — UART driver
- Baudrate derived from the actual clock tree values
- Alternate function pin config
- `uart_init()`, `uart_write()`, `uart_read()`
- `printf` redirect over UART (optional but useful for debugging)

### ⬜ Level 5 — ADC + DMA
- ADC continuous mode
- DMA circular buffer: `uint16_t adc_buffer[256]`
- CPU does nothing during acquisition

### ⬜ Level 6 — Non-blocking UART
- Ring buffer for TX
- Interrupt-driven transmission
- No blocking I/O anywhere in the system

### ⬜ Level 7 — FreeRTOS
- Three tasks: Acquisition, Processing, Communication
- Queues, mutexes, semaphores for inter-task sync
- No `vTaskDelay` used as a substitute for proper synchronization

### ⬜ Level 8 — Full system
- Watchdog
- CPU load monitoring
- System log over UART
- Complete pipeline running end to end

---

## MISRA-C

I follow MISRA-C mandatory rules as a way to catch classes of bugs early. The main habits:

- `stdint.h` types everywhere (`uint32_t`, `uint16_t`, etc.) — no bare `int` in driver code
- Explicit casts, no silent integer promotions
- No function-like macros where an `inline` function works
- Functions do one thing
- Global variables only when genuinely necessary

Checked with cppcheck:
```bash
cppcheck --addon=misra.py src/
```

---

## Debugging

The whole project is built to be inspectable at register level. Typical session:

```bash
# Terminal 1 — OpenOCD
openocd -f interface/stlink.cfg -f target/stm32f4x.cfg

# Terminal 2 — GDB
arm-none-eabi-gdb build/firmware.elf
target remote :3333

# Check the clock actually locked
x/wx 0x40023800   # RCC_CR      → bit 25 (PLLRDY) should be set
x/wx 0x40023804   # RCC_PLLCFGR → verify PLLM/N/P values
x/wx 0x40023808   # RCC_CFGR    → bits 2:3 (SWS) should read 0b10 (PLL)
```

---

## References

### Hardware Documentation
- [STM32F411xC/E Reference Manual (RM0383)](https://www.st.com/resource/en/reference_manual/rm0383-stm32f411xce-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [STM32F411xC/E Datasheet](https://www.st.com/resource/en/datasheet/stm32f411ce.pdf)
- [STM32F411E-DISCO User Manual (UM1842)](https://www.st.com/resource/en/user_manual/um1842-discovery-kit-with-stm32f411ve-mcu-stmicroelectronics.pdf)
- [STM32 Cortex-M4 Programming Manual (PM0214)](https://www.st.com/resource/en/programming_manual/pm0214-stm32-cortexm4-mcus-and-mpus-programming-manual-stmicroelectronics.pdf)

### Software & Standards
- [ARM Cortex-M4 Technical Reference Manual](https://developer.arm.com/documentation/100166/latest/)
- [FreeRTOS Real Time Kernel Documentation](https://www.freertos.org/Documentation/RTOS_book.html)
- [MISRA-C:2012 Guidelines](https://www.misra.org.uk/)

