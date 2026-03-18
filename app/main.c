#include "system_clock.h"
#include <stdint.h>

/* Use explicit 'U' suffix for unsigned constants (MISRA compliance) */
/* .data section: Reset_Handler must copy 0xDEADBEEF from Flash to SRAM */
uint32_t var_init = 0xDEADBEEFU;

/* .bss section: Reset_Handler must zero-fill this in SRAM */
uint32_t var_uninit;

/* .rodata section: constant, lives permanently in Flash */
const uint32_t var_ct = 0xCAFEFEEDU;

int main(void)
{
    /* If we reach here, the stack pointer is already pointing to RAM */
    /* and both .data and .bss have been correctly initialized        */

    /* Initialize the system clock at 96 MHz */
    system_clock_init();

    for (;;)
    {
        /* Modify vars so changes are visible in GDB */
        var_uninit++;

        if (var_uninit > 100U)
        {
            var_uninit = 0U;
        }
    }

    return 0;
}


