# Level 0 - Startup and Linker

### A brief overview of university fundamentals

#### Cortex-M registers

> Reference: PM0214, section 2.1.3

- **R0 - R12** -> General purpose. 
- **SP -> Stack Pointer (R13).** Points to the current top of the stack in RAM. This is the very first value the CPU fetches from memory (Address `0x0000 0000`). 
- **LR -> Link Register (R14).** Stores the return address when a function or subroutine is called. 
- **PC -> Program Counter (R15).** The pointer to the next instruction to be executed. On reset, the CPU loads it with the address of the `Reset_Handler`. 

#### The vector table

> Reference: PM0214, section 2.3.4

The vector table is a reserved map at the beginning of the memory space that contains the addresses of all exception handlers (Reset, NMI, HardFault) and interrupts. 

- **Initial SP value:** Located at offset `0x0000`. 
- **Reset vector:** Located at offset `0x0004`. It holds the address of the `Reset_Handler`. 

#### Stack vs. Heap: SRAM Layout

In the STM32F411, both reside in SRAM. The heap grows upwards from the end of the static data sections (`.bss`), while the stack grows downwards from the highest SRAM address (top of the stack). 

If the stack and heap meets, a stack-heap collision occurs, leading to memory corruption and system failure. 

---

### Memory mapping

> Reference: RM0383, section 2.4, table 3

Flash (512KB) - `0x0800 0000`
SRAM  (128KB) - `0x2000 0000`

---

### The boot sequence

> Reference: RM0383, section 2.4

The boot process is essentially a hardware-controlled reset sequence:

- **Hardware Mapping:** The BOOT0 pin (physical jumper) determines if the Flash is aliased to `0x0000 0000`.
- **SP Initialization:** The CPU loads the MSP with the value at `0x0000 0000`.
- **PC Initialization:** The CPU loads the PC with the address at `0x0000 0004`.
- **Execution:** The CPU jumps to the Reset Handler to initialize the C runtime (copying `.data` and clearing `.bss`).

**The C Runtime Initialization (CRT)**

Once the `Reset_Handler` starts, it must prepare the SRAM before `main()` can run:

- **Copying `.data`:** Moves initialized global and static variables from Flash (LMA) to SRAM (VMA).
- **Clearing `.bss`:** Sets all uninitialized global variables to zero in SRAM.
- **Branch to main:** Once the memory enviroment is safe, it calls the `main()` function.

---

### Reset/Boot sequence flow simplified

Hardware Reset → Fetch SP (0x0) → Fetch PC (0x4) → Reset_Handler → [Copy `.data`] → [Zero `.bss`] → main().

---

### The Reset Handler and memory initialization

The Reset Handler is the entry point of the software. Its primary responsibility is the **C-Runtime Initialization**, specifically moving the `.data` section.

- **Why move it?:** Global variables with initial values are stored in non-volatile Flash (to survive power-off) but must be accessed in SRAM to allow read/write operations.
- **LMA (Load Memory Address):** The location in Flash where the data is stored.
- **VMA (Virtual Memory Address):** The location in SRAM where the data will reside during execution.
- **The Copy Loop:** The `Reset_Handler` uses symbols provided by the linker script to perform a memory-to-memory copy from Flash to SRAM before calling `main()`.

---

### Linker terminology

- **Linker Script Language:** A command language (DSL: Domain Specific Language) for the GNU linker (`ld`) used to describe the memory layout of the target system.
- **Alignment(`ALIGN(4)`):** Ensures that the addresses start on a 32-bit boundary, matching the Cortex-M4 bus width for optimal performance.
- **Standard Sections**:
    - `.text`: Executable code and read-only data.
    - `.data`: Global variables with a non-zero initial value.
    - `.bss`: Global variables initialized to zero, allocated in RAM but not stored in Flash. 

In this project, the linker is called `memory.ld`.

---

### Startup file (`startup.c`)

The startup files bridges the hardware reset sequence and the application's `main()`. It runs before any user code and is responsible for making the C runtime enviroment safe. 
Responsibilities:
- Define the interrupt vector table. 
- Implement `Reset_Handler`.
- Provide default exception handlers, so that any undhandled exception has a safe fallback. 

#### Reset_Handler runtime initialization


| Step           | Action                    | Why                                       |
|----------------|---------------------------|-------------------------------------------|
|  Copy `.data`  | Flash (LMA) -> SRAM (VMA) | Global variables must be writable         |
|  Zero `.bss`   | SRAM -> 0x0000 0000       | C standard requires zero-initialization   |
|  Call `main()` | Branch to application     | Memory enviroment is now safe             |
|  `while(1)`    | Infinite loop             | Prevents executing garbage after `main()` |

---

### MISRA-C Guidelines & Deviations

MISRA-C:2012 rules are followed as a coding discipline where practical.
Full compliance is not claimed — this is a learning project, not a certified product.

**Deviations:**

**Rule 18.3 — Pointer comparison**
Comparing `pDst < _edata` where both are independent linker symbols.
Mitigated by declaring linker symbols as arrays (`extern uint32_t _sdata[]`),
which groups the memory region as a single logical object.

**Rule 11.1 — Pointer casting in vector table**
Casting `Reset_Handler` to `uint32_t` is unavoidable — the Cortex-M
architecture requires a flat array of addresses at boot.

**Rule 14.3 — Invariant control expression**
`while (1)` triggers a warning for a constant condition.
Use `for (;;)` instead, which is the idiomatic MISRA-safe infinite loop.

---

### Compilation

The project uses a handwritten `Makefile`.


#### Makefile structure

```
all
 ├── firmware.elf
 │    ├── build/core/setup.o   ← core/setup.c
 │    └── build/app/main.o     ← app/main.c
 └── firmware.bin
      └── objcopy firmware.elf → firmware.bin
```


#### Output files

- `firmware.elf` -> Executable and linkable format (ELF). Standard binary GCC format. Contains code, data, section metadata and debug symbols. Used by GDB. 
- `firmware.bin` -> Raw byte dump of the flash-able sections. No headers, no symbols.
  Produced by `objcopy` from the `.elf`. Written to `0x08000000` by `st-flash`.
- `firmware.map` -> Symbol and section map, useful to verify LMA/VMA placement. 

---

### Debugging & Verification

#### 1. Compilation check

Running `make` produces:

```bash
arm-none-eabi-size build/firmware.elf
   text    data     bss     dec     hex    filename
    144      20       4     168      a8    build/firmware.elf
```

- `bss = 4 bytes` — exactly `sizeof(uint32_t)` for `var_uninit`. Correct.
- `data = 20 bytes` — `var_init` (4 bytes) plus the vector table entries copied to RAM.

The RWX warning is expected in bare-metal — the linker sees SRAM as readable, writable, and executable. Not a problem.


#### 2. Section placement check

```bash
arm-none-eabi-objdump -h build/firmware.elf
```

```
Idx  Name        Size      VMA       LMA
  0  .isr_vector 00000010  08000000  08000000  ← vector table at start of Flash
                 CONTENTS, ALLOC, LOAD, DATA
  1  .text       00000090  08000010  08000010  ← code in Flash
                 CONTENTS, ALLOC, LOAD, READONLY, CODE
  2  .data       00000004  20000000  080000a0  ← VMA in SRAM, LMA in Flash ✓
                 CONTENTS, ALLOC, LOAD, DATA
  3  .bss        00000004  20000004  080000a4  ← SRAM only, no Flash storage ✓
                 ALLOC
```

Key observations:
- `.isr_vector` starts at `0x08000000`: the linker script placed it correctly.
- `.data` has VMA in SRAM (`0x20000000`) and LMA in Flash (`0x080000a0`) —
  `Reset_Handler` will copy from LMA to VMA at boot.
- `.bss` has no `CONTENTS` flag: no Flash storage, zeroed at runtime by `Reset_Handler`.


#### 3. Flash & Debug

First, verify the ST-LINK is detected:

```bash
lsusb | grep -i "st-link"
# Bus 001 Device 007: ID 0483:3748 STMicroelectronics ST-LINK/V2
```

Launch OpenOCD in one terminal:

```bash
openocd -f interface/stlink.cfg -f target/stm32f4x.cfg
# Info : [stm32f4x.cpu] Cortex-M4 r0p1 processor detected
# Info : [stm32f4x.cpu] target has 6 breakpoints, 4 watchpoints
# Info : Listening on port 3333 for gdb connections
```

Flash and debug in a second terminal:
```bash
arm-none-eabi-gdb build/firmware.elf
(gdb) target remote :3333
(gdb) monitor reset halt
(gdb) load
(gdb) monitor reset init
```

`load` writes each section to its LMA in Flash:

```
Loading section .isr_vector, size 0x10 lma 0x8000000
Loading section .text, size 0x90 lma 0x8000010
Loading section .data, size 0x4 lma 0x80000a0
Start address 0x08000018, load size 164
```

After `monitor reset init`, the CPU halts at the reset vector:

```
xPSR: 0x01000000 pc: 0x08000018 msp: 0x20020000
```

- `pc = 0x08000018` — CPU is at `Reset_Handler`, not yet at `main()`.
- `msp = 0x20020000` — Stack pointer correctly loaded from `_estack` (top of RAM).


#### 4. Verification

Set a breakpoint at `main()` and verify the memory layout:

```bash
(gdb) b main
(gdb) continue

(gdb) p/x var_init        # $1 = 0xdeadbeef  — .data copied from Flash to SRAM ✓
(gdb) p var_uninit        # $2 = 0            — .bss zeroed by Reset_Handler ✓

(gdb) info address var_init    # 0x20000000  — SRAM ✓
(gdb) info address var_uninit  # 0x20000004  — SRAM ✓
(gdb) info address var_ct      # 0x0800009c  — Flash (.rodata) ✓
```

All three sections are where the linker script placed them.
Level 0 complete: `main()` runs with a fully initialized C runtime.
