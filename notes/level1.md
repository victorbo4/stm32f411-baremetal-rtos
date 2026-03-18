# Level 1 - Clock System

### Clock fundamentals & terminology

> Reference: RM0383, section 6.2 

Before writing to the `RCC` (Reset and Clock Control) registers, it is crucial to understand how the STM32 routes and scales its clock signals. 
The clock system is essentially a pipeline: **Sources → Multiplier → Switch → Buses → Peripherals**.

#### 1. Clock Sources (The Origins)

The microcontroller needs a baseline heartbeat to function. There are two primary high-speed sources:
* **HSI (High-Speed Internal):** A 16 MHz internal RC oscillator. It is less precise and sensitive to temperature variations, but it requires no external components. **Crucially, this is the default clock the CPU uses immediately after a hardware reset.**
* **HSE (High-Speed External):** An external crystal oscillator (typically 4 to 26 MHz, like the 8 MHz crystal often found on Discovery boards). It provides a highly accurate clock, which is strictly required if you intend to use sensitive peripherals like USB.

#### 2. The Multiplier: PLL (Phase-Locked Loop)

Since the Cortex-M4 core in the STM32F411 can run up to 100 MHz, but our sources are only 16 MHz (HSI) or 8 MHz (HSE), we need a hardware accelerator to multiply the frequency. This is the PLL.
The main PLL takes an input clock and passes it through a series of configurable stages:
* `/M` (Division factor): Scales down the input clock to a safe baseline (usually 1-2 MHz).
* `*N` (VCO Multiplier): Multiplies the baseline clock to a very high internal frequency.
* `/P` (Main System Division): Divides the high frequency back down to our target System Clock (e.g., 96 MHz).
* `/Q` (USB/SDIO Division): A separate divider specifically to generate a strict 48 MHz for USB hardware.

#### 3. The Switch and the Core

* **SW (System Clock Switch):** A multiplexer controlled via the `RCC_CFGR` register. It allows you to select on-the-fly which source drives the entire chip. You can switch between HSI, HSE, or the PLL output.
* **SYSCLK (System Clock):** The main clock signal resulting from the SW selection. If we select the PLL, SYSCLK becomes 96 MHz.

#### 4. The Distribution Network: Buses & Prescalers

We cannot feed 96 MHz directly into every peripheral, as many cannot handle such high speeds. The STM32 routes the `SYSCLK` through an internal highway system made of buses, using **Prescalers** (hardware dividers) to lower the speed for specific zones.

* **AHB (Advanced High-performance Bus):** The main freeway. It connects the CPU (Cortex-M4), Flash memory, SRAM, and DMA. Its clock is called **HCLK**. Usually, its prescaler is set to `/1`, meaning it runs at the full `SYSCLK` speed (96 MHz).
* **APB (Advanced Peripheral Bus):** The local roads for standard peripherals (Timers, UART, SPI, I2C). The STM32F411 splits this into two domains with different hardware limits:
    * **APB1 (Low-speed peripheral bus):** Hardware maximum is **50 MHz**. If our `SYSCLK` is 96 MHz, we *must* configure the APB1 prescaler to at least `/2` (yielding 48 MHz). Failing to do so will cause peripherals on this bus to fail.
    * **APB2 (High-speed peripheral bus):** Hardware maximum is **100 MHz**. It can safely run at the full 96 MHz `SYSCLK` by setting its prescaler to `/1`.

#### The Target Pipeline for 96 MHz

`HSI (16 MHz) → PLL (M, N, P config) → SW (Select PLL) → SYSCLK (96 MHz) → AHB (/1) → APB1 (/2) & APB2 (/1)`

---

### The PLL Mathematics

To reach our target 96 MHz from the 16 MHz HSI, we need to configure the main PLL. The formula governing the output frequency is derived from the internal hardware stages:

`f_sysclk = (f_hsi / M) * (N / P)`


#### Clock Chain Stages

> Reference: RM0383, Section 6.3.2 (RCC_PLLCFGR)

Where the internal stages are:
1. VCO Input: `f_vcoin = f_hsi / M`
2. VCO Output: `f_vcoout = f_vcoin * N`
3. Main System Output: `f_sysclk = f_vcoout / P`
4. USB/SDIO Output: `f_usb = f_vcoout / Q`


#### The Hardware Constraints

> Reference: STM32F411xC/E Datasheet, Section 6.3.11 (PLL characteristics)

We cannot choose arbitrary numbers. The configuration must respect the physical limits of the silicon to ensure the PLL locks and remains stable:

- `f_hsi`: 16 MHz (fixed internal source).
- `M` (division factor): between 2 and 63. Therefore, `f_vcoin` must be between 1 MHz and 2 MHz (ST recommends 2 MHz to limit jitter).
- `N` (multiplication factor): between 50 and 432. Therefore, `f_vcoout` must be between 192 MHz and 432 MHz.
- `P` (main divider): can only be 2, 4, 6, or 8. Therefore, `f_sysclk` cannot exceed 100 MHz on the STM32F411.
- `Q` (USB divider): between 2 and 15. The USB peripheral strictly requires 48 MHz.


#### Calculating for 96 MHz

**Step 1: Calculate `M` (VCO Input)**
We divide the HSI to reach the minimum allowed input frequency for simpler math.
`M = 16 -> f_vcoin = 16 MHz / 16 = 1 MHz`
*(Valid: 1 MHz is within the 1-2 MHz range).*

**Step 2: Choose `P` & Calculate `N`**
We target `f_sysclk = 96 MHz`. We choose the highest performance divider `P = 2`.
To find `N`, we work backwards from the required VCO output:

`f_vcoout = f_sysclk * P = 96 MHz * 2 = 192 MHz`

Now, solve for the multiplier `N`:
`N = f_vcoout / f_vcoin = 192 MHz / 1 MHz = 192`
*(Valid: N=192 is the minimum allowed value for f_vcoout stability).*

**Step 3: Calculate `Q` (USB Clock)**
To ensure future compatibility with USB/SDIO, we need 48 MHz.

`Q = f_vcoout / f_usb = 192 / 48 = 4`
*(Valid: Q=4 is within the allowed range).*


#### Final Register Values for RCC_PLLCFGR:

- PLLM = 16
- PLLN = 192
- PLLP = 0 (Internal bit value `00` maps to divisor 2)
- PLLQ = 4

---

### Flash Memory Latency (Wait States)

> Reference: RM0383, Section 3.4.1 (Relation between CPU clock frequency and Flash memory read time)

When the CPU frequency (SYSCLK) increases, the processor becomes significantly faster than the physical Flash memory. If the Cortex-M4 tries to fetch instructions at 96 MHz without any adjustments, the Flash cannot keep up, leading to a system crash (HardFault) before `main()` even executes.

To prevent this, we must configure **Wait States (WS)** in the Flash Access Control Register (`FLASH_ACR`). A Wait State is an intentional delay (measured in CPU cycles) that gives the Flash memory enough time to output the requested data onto the bus.

#### The Hardware Constraints

The required number of Wait States depends on two factors: the CPU clock frequency (HCLK) and the operating voltage (VDD). 

For our specific setup on the STM32F411E-DISCO board:
- **Operating Voltage:** 3.3V
- **Target HCLK:** 96 MHz


According to the RM0383 table for 2.7V - 3.6V voltage range:
- 0 < HCLK <= 30 MHz: 0 WS (1 CPU cycle)
- 30 < HCLK <= 60 MHz: 1 WS (2 CPU cycles)
- 60 < HCLK <= 90 MHz: 2 WS (3 CPU cycles)
- **90 < HCLK <= 100 MHz: 3 WS (4 CPU cycles)**

#### Configuration Rule

Because our target is 96 MHz, we fall into the highest bracket. **Before** we switch the system clock source to the PLL, we must write `0011` (3 WS) into the `LATENCY` bits of the `FLASH_ACR` register. If we switch the clock to 96 MHz before configuring the Flash latency, the system will immediately freeze.

---

### Implementation: system_clock.c

The system clock configuration follows a strict hardware synchronization sequence to transition from the 16 MHz HSI to the 96 MHz PLL output without crashing the CPU.

**Key Steps in `system_clock_init()`:**

1. **Enable HSI:** Ensure the internal 16 MHz oscillator is ON and stable.
2. **Flash Configuration:** Set Latency to 3 Wait States (WS) and enable Instruction/Data caches **before** increasing frequency.
3. **PLL Setup:** Configure M, N, P, and Q factors in `RCC_PLLCFGR` using HSI as the source.
4. **PLL Lock:** Enable the PLL and poll the `PLLRDY` bit until the hardware achieves a frequency lock.
5. **Bus Prescalers:** Set APB1 to `/2` (48 MHz) to stay within the 50 MHz limit defined in the datasheet.
6. **Clock Switch:** Transition `SYSCLK` to the PLL output and verify the switch via the `SWS` bits in `RCC_CFGR`.

---

### Hardware Verification (GDB Debugging)

After flashing the firmware, we must verify that the hardware registers reflect our manual configuration. Using `arm-none-eabi-gdb` connected via `OpenOCD`, we perform the following checks:

#### 1. PLL Status Check

We inspect the `RCC_CR` register (Address: `0x40023800`) to ensure the PLL is enabled and stable.

```gdb
(gdb) x/wt 0x40023800
```
Expected Output: Bit 25 (`PLLRDY`) and Bit 24 (`PLLON`) must be `1`.
Success: The PLL has achieved a frequency lock.

#### 2. System Clock Source Check

We inspect the `RCC_CFGR` register (Address: `0x40023808`) to verify the CPU is actually using the PLL as its clock source.

```gdb
(gdb) x/wx 0x40023808
```

Expected Output: Bits 3:2 (`SWS`) must be `10` (binary).
Note: If the hex value ends in `8`, `9`, `A`, or `B`, the switch was successful.

#### 3. Flash Latency Check

We inspect the `FLASH_ACR` register (Address: `0x40023C00`) to confirm the Wait States are active, preventing CPU crashes at 96 MHz.

```gdb
(gdb) x/wx 0x40023C00
```

Expected Output: Bits 2:0 (`LATENCY`) must be `011` (3 Wait States).
Additional: Bits 10:8 should be `111` if Prefetch and Caches are enabled.

#### 4. Bus Speed Verification (APB1)

In the same `RCC_CFGR` register, we check the APB1 prescaler.
Expected Output: Bits 12:10 (`PPRE1`) must be `100` (binary), indicating a division by 2.
Safety: This ensures the APB1 bus runs at 48 MHz, staying below the 50 MHz hardware limit.

---

## MISRA-C:2012 Compliance Summary

The following deviations are intentional and required for bare-metal operation:

| Rule | Description | Justification |
| :--- | :--- | :--- |
| **11.4 / 11.6** | Integer to pointer conversion | Required to access Memory Mapped I/O (MMIO) registers (RCC, Flash). |
| **8.11** | External arrays with incomplete type | Required to access Linker Script symbols (`_sdata`, `_sbss`, etc.). |
| **12.2** | Shift count and value types | Used explicitly for bitmask clarity in register configuration. |
| **8.7** | External linkage | `Reset_Handler` and global variables must be visible to the hardware/linker. |


