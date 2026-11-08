/*
*****************************************************************************
**
**  File        : stm32_flash_g474xe.ld
**
**  Abstract    : Linker script for STM32G473xE/STM32G474xE devices
**                512 KB flash, 96 KB SRAM1+SRAM2 and 32 KB CCM SRAM
**
*****************************************************************************
*/

/* Stack & Heap sizes */
_Min_Heap_Size = 0;
_Min_Stack_Size = 0x2000;

/* Entry Point */
ENTRY(Reset_Handler)

/*
0x08000000 to 0x0807FFFF  512K full flash,
0x08000000 to 0x08003FFF   16K isr vector, startup code,
0x08004000 to 0x08007FFF   16K config,                  // 8 x 2K pages (dual bank) or 4 x 4K pages (single bank)
0x08008000 to 0x0807FFFF  480K firmware,

SRAM1 (80K) and SRAM2 (16K) are contiguous at 0x20000000.
CCM SRAM (32K) is aliased right after them at 0x20018000; it holds the stack and
FASTRAM (as on the F405, INAV keeps DMA buffers out of FASTRAM).
*/

MEMORY
{
    FLASH (rx)        : ORIGIN = 0x08000000, LENGTH = 16K
    FLASH_CONFIG (r)  : ORIGIN = 0x08004000, LENGTH = 16K
    FLASH1 (rx)       : ORIGIN = 0x08008000, LENGTH = 480K

    RAM (rwx)         : ORIGIN = 0x20000000, LENGTH = 96K
    CCM (rwx)         : ORIGIN = 0x20018000, LENGTH = 32K
    MEMORY_B1 (rx)    : ORIGIN = 0x60000000, LENGTH = 0K
}

REGION_ALIAS("STACKRAM", CCM)
REGION_ALIAS("FASTRAM", CCM)

INCLUDE "stm32_flash_split.ld"
