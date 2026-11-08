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

/*
 * Config storage in internal flash for STM32G4 (G473/G474).
 *
 * The G47x flash can run in one of two layouts, selected by the DBANK option bit:
 *   DBANK = 1 (factory default): 2 banks x 128 pages x 2 KB
 *   DBANK = 0                  : 1 bank  x 128 pages x 4 KB
 * The layout is detected at runtime so either option-byte setting works.
 * Programming granularity is a 64-bit double word.
 */

#include <string.h>
#include "platform.h"
#include "common/utils.h"
#include "drivers/system.h"
#include "config/config_streamer.h"

#if defined(STM32G4) && !defined(CONFIG_IN_RAM) && !defined(CONFIG_IN_EXTERNAL_FLASH)

STATIC_ASSERT(CONFIG_STREAMER_BUFFER_SIZE == sizeof(uint64_t), config_streamer_buffer_must_be_a_double_word);

static bool flashIsDualBank(void)
{
#if defined(FLASH_OPTR_DBANK)
    return (FLASH->OPTR & FLASH_OPTR_DBANK) != 0;
#else
    return false;
#endif
}

static uint32_t flashPageSize(void)
{
    return flashIsDualBank() ? 0x800U : 0x1000U;
}

static void getFLASHPageForEEPROM(uint32_t address, uint32_t *bank, uint32_t *page)
{
    uint32_t offset = address - FLASH_BASE;

    if (flashIsDualBank()) {
        // In dual bank mode the bank size is half of the device flash
        const uint32_t bankSize = (MCU_FLASH_SIZE * 1024U) / 2U;
        if (offset < bankSize) {
            *bank = FLASH_BANK_1;
        } else {
            *bank = FLASH_BANK_2;
            offset -= bankSize;
        }
    } else {
        *bank = FLASH_BANK_1;
    }

    *page = offset / flashPageSize();
}

void config_streamer_impl_unlock(void)
{
    HAL_FLASH_Unlock();
    // Clear any stale error flags left from previous operations
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);
}

void config_streamer_impl_lock(void)
{
    HAL_FLASH_Lock();
}

int config_streamer_impl_write_word(config_streamer_t *c, config_streamer_buffer_align_type_t *buffer)
{
    if (c->err != 0) {
        return c->err;
    }

    if (c->address % flashPageSize() == 0) {
        FLASH_EraseInitTypeDef EraseInitStruct = {
            .TypeErase = FLASH_TYPEERASE_PAGES,
            .NbPages = 1,
        };
        getFLASHPageForEEPROM(c->address, &EraseInitStruct.Banks, &EraseInitStruct.Page);

        uint32_t pageError;
        const HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&EraseInitStruct, &pageError);
        if (status != HAL_OK) {
            return -1;
        }
    }

    uint64_t data;
    memcpy(&data, buffer, sizeof(data));

    const HAL_StatusTypeDef status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, c->address, data);
    if (status != HAL_OK) {
        return -2;
    }

    c->address += CONFIG_STREAMER_BUFFER_SIZE;
    return 0;
}

#endif
