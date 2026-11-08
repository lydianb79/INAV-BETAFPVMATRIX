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
 * STM32G47x SPI pin alternate function lookup table.
 *
 * Usage: SPI_PIN_AF_HELPER(3, PB5) expands to SPI_PIN_AF_SPI3_PB5.
 * Alternate functions are from the STM32G474 datasheet (DS12288),
 * Table 13 "Alternate function" (AF5 for SPI1/2/4, AF6 for SPI3,
 * with a few AF5 SPI3 exceptions on port C/G).
 */

#pragma once

#include "common/utils.h"

#define SPI_PIN_AF_HELPER(spi, pin)  CONCAT4(SPI_PIN_AF_SPI, spi, _, pin)

/* SPI1 */
#define SPI_PIN_AF_SPI1_PA5    GPIO_AF5_SPI1   // SCK
#define SPI_PIN_AF_SPI1_PA6    GPIO_AF5_SPI1   // MISO
#define SPI_PIN_AF_SPI1_PA7    GPIO_AF5_SPI1   // MOSI
#define SPI_PIN_AF_SPI1_PB3    GPIO_AF5_SPI1   // SCK
#define SPI_PIN_AF_SPI1_PB4    GPIO_AF5_SPI1   // MISO
#define SPI_PIN_AF_SPI1_PB5    GPIO_AF5_SPI1   // MOSI

/* SPI2 */
#define SPI_PIN_AF_SPI2_PB13   GPIO_AF5_SPI2   // SCK
#define SPI_PIN_AF_SPI2_PB14   GPIO_AF5_SPI2   // MISO
#define SPI_PIN_AF_SPI2_PB15   GPIO_AF5_SPI2   // MOSI
#define SPI_PIN_AF_SPI2_PF1    GPIO_AF5_SPI2   // SCK
#define SPI_PIN_AF_SPI2_PA10   GPIO_AF5_SPI2   // MISO
#define SPI_PIN_AF_SPI2_PA11   GPIO_AF5_SPI2   // MOSI

/* SPI3 */
#define SPI_PIN_AF_SPI3_PB3    GPIO_AF6_SPI3   // SCK
#define SPI_PIN_AF_SPI3_PB4    GPIO_AF6_SPI3   // MISO
#define SPI_PIN_AF_SPI3_PB5    GPIO_AF6_SPI3   // MOSI
#define SPI_PIN_AF_SPI3_PC10   GPIO_AF6_SPI3   // SCK
#define SPI_PIN_AF_SPI3_PC11   GPIO_AF6_SPI3   // MISO
#define SPI_PIN_AF_SPI3_PC12   GPIO_AF6_SPI3   // MOSI

/* SPI4 */
#define SPI_PIN_AF_SPI4_PE2    GPIO_AF5_SPI4   // SCK
#define SPI_PIN_AF_SPI4_PE5    GPIO_AF5_SPI4   // MISO
#define SPI_PIN_AF_SPI4_PE6    GPIO_AF5_SPI4   // MOSI
#define SPI_PIN_AF_SPI4_PE12   GPIO_AF5_SPI4   // SCK
#define SPI_PIN_AF_SPI4_PE13   GPIO_AF5_SPI4   // MISO
#define SPI_PIN_AF_SPI4_PE14   GPIO_AF5_SPI4   // MOSI
