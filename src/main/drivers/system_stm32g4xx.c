/*
 * This file is part of INAV.
 *
 * INAV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * INAV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with INAV.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "platform.h"

#include "drivers/accgyro/accgyro_mpu.h"
#include "drivers/persistent.h"
#include "drivers/exti.h"
#include "drivers/nvic.h"
#include "drivers/system.h"

void enableGPIOPowerUsageAndNoiseReductions(void)
{
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();
}

bool isMPUSoftReset(void)
{
    if (cachedRccCsrValue & RCC_CSR_SFTRSTF)
        return true;
    else
        return false;
}

uint32_t systemBootloaderAddress(void)
{
    // STM32G47x system memory (ROM bootloader), AN2606
    return 0x1FFF0000;
}

void systemInit(void)
{
    // Configure NVIC preempt/priority groups
    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITY_GROUPING);

    // cache RCC->CSR value to use it in isMPUSoftReset() and others
    cachedRccCsrValue = RCC->CSR;

    enableGPIOPowerUsageAndNoiseReductions();

    // Disable the UCPD dead battery pull-downs on PB4/PB6 (CC1/CC2), which would
    // otherwise load SPI3 MISO (PB4) and the motor/LED pins on PB6.
    // See AN5093 6.3.3. Do not touch the USB standby control: that breaks
    // the jump to the ROM bootloader.
#if defined(PWR_CR3_UCPD_DBDIS)
    HAL_PWREx_DisableUSBDeadBatteryPD();
#endif

    // Init cycle counter
    cycleCounterInit();

    // SysTick
    HAL_SYSTICK_Config(SystemCoreClock / 1000);
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
}
