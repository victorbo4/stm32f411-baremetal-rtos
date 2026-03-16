/* memory.ld */

ENTRY(Reset_Handler)

/* End of RAM to stack pointer */
_estack = 0x20020000;   /* 128KB SRAM */


MEMORY
{
    FLASH (rx)      : ORIGIN = 0x08000000, LENGTH = 512K
    SRAM (xrw)      : ORIGIN = 0x20000000, LENGTH = 128K
}

SECTIONS
{
    /* 1. Vector table MUST be placed at the beginning of Flash */
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))    /* keep the table even if it appears unreferenced */
        . = ALIGN(4);
    } > FLASH

    /* 2. Program code section */
    .text :
    {
        . = ALIGN(4);
        *(.text)                /* all .text code sections */
        *(.text*)               /* all .text sub-sections (e.g. .text.func) */
        *(.rodata)              /* read-only data (constants, string literals) */
        *(.rodata*)
        . = ALIGN(4);
        _etext = .;             /* symbol marking the end of code in Flash */
    } > FLASH

    /* 3. Symbol for the Load Memory Access (LMA) of the .data section */
    /* This is where the the initializer values are stored in Flash */
    _sidata = LOADADDR(.data);

    /* 4. Initialized data section (.data)         */
    /*  VMA (runtime address) is SRAM           */
    /*  LMA (load/storage address) is Flash     */
    /*  The startup code copies the data from LMA to VMA at boot */
    .data :
    {
        . = ALIGN(4);
        _sdata = .;             /* symbol marking the start of .data in RAM */
        *(.data)
        *(.data*)
        . = ALIGN(4);
        _edata = .;             /* symbol marking the end of .data in RAM */
    } > SRAM AT > FLASH         /* place VMA in SRAM, but store initializers in Flash */

    /* 5. Uninitialized data section (.bss)                    */
    /*  No storage needed in Flash: startup code zeroes     */
    /*  this entire region before main() is called          */
    .bss :
    {
        _sbss = .;              /* symbol marking the start of .bss */
        *(.bss)
        *(.bss*)
        *(COMMON)               /* common symbols (tentative definitions in C) */
        . = ALIGN(4);
        _ebss = .;              /* symbol marking the end of .bss */
    } > SRAM
}

