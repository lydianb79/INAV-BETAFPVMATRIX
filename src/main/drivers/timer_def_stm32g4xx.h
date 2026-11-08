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
 * STM32G47x timer definitions.
 *
 * The AF table is taken from Betaflight (src/platform/STM32/timer_def.h, STM32G4 section),
 * which follows DS12288 Table 13.
 *
 * DMA: the G4 has DMA1 and DMA2 with 8 channels each and a DMAMUX that can route any
 * request to any channel. To stay compatible with the stream based DMA_TAG used by the
 * other STM32 ports, the "stream" field holds the channel number minus one (0..7) and
 * the "channel" field holds the DMAMUX request.
 * dmavar 0..7 = DMA1 channel 1..8, dmavar 8..15 = DMA2 channel 1..8.
 */

#pragma once

#define timerDMASafeType_t  uint32_t

// Missing from some G4 HAL versions
#ifndef GPIO_AF12_TIM1
#define GPIO_AF12_TIM1      ((uint8_t)0x0C)  /* TIM1 Alternate Function mapping */
#endif

#define DEF_TIM_DMAMAP__D(dma, stream, request) DMA_TAG(dma, stream, request)
#define DEF_TIM_DMAMAP__NONE                    DMA_NONE

#define DEF_TIM(tim, ch, pin, usage, flags, dmavar) {   \
    tim,                                                \
    IO_TAG(pin),                                        \
    DEF_TIM_CHNL_ ## ch,                                \
    DEF_TIM_OUTPUT(ch) | flags,                         \
    IOCFG_AF_PP,                                        \
    DEF_TIM_AF(TCH_## tim ## _ ## ch, pin),             \
    usage,                                              \
    DEF_TIM_DMAMAP(dmavar, tim ## _ ## ch)              \
}

// AF mappings
#define DEF_TIM_AF(timch, pin)        CONCAT(DEF_TIM_AF__, DEF_TIM_AF__ ## pin ## __ ## timch)
#define DEF_TIM_AF__D(af_n, tim_n)    GPIO_AF ## af_n ## _TIM ## tim_n

/* G4 DMA channel mappings, D(DMAx, channel - 1, request) */
#define DEF_TIM_DMA_FULL(req) \
    D(1, 0, req), D(1, 1, req), D(1, 2, req), D(1, 3, req), D(1, 4, req), D(1, 5, req), D(1, 6, req), D(1, 7, req), \
    D(2, 0, req), D(2, 1, req), D(2, 2, req), D(2, 3, req), D(2, 4, req), D(2, 5, req), D(2, 6, req), D(2, 7, req)

#define DEF_TIM_DMA__BTCH_TIM1_CH1      DEF_TIM_DMA_FULL(DMA_REQUEST_TIM1_CH1)
#define DEF_TIM_DMA__BTCH_TIM1_CH2      DEF_TIM_DMA_FULL(DMA_REQUEST_TIM1_CH2)
#define DEF_TIM_DMA__BTCH_TIM1_CH3      DEF_TIM_DMA_FULL(DMA_REQUEST_TIM1_CH3)
#define DEF_TIM_DMA__BTCH_TIM1_CH4      DEF_TIM_DMA_FULL(DMA_REQUEST_TIM1_CH4)
#define DEF_TIM_DMA__BTCH_TIM2_CH1      DEF_TIM_DMA_FULL(DMA_REQUEST_TIM2_CH1)
#define DEF_TIM_DMA__BTCH_TIM2_CH2      DEF_TIM_DMA_FULL(DMA_REQUEST_TIM2_CH2)
#define DEF_TIM_DMA__BTCH_TIM2_CH3      DEF_TIM_DMA_FULL(DMA_REQUEST_TIM2_CH3)
#define DEF_TIM_DMA__BTCH_TIM2_CH4      DEF_TIM_DMA_FULL(DMA_REQUEST_TIM2_CH4)
#define DEF_TIM_DMA__BTCH_TIM3_CH1      DEF_TIM_DMA_FULL(DMA_REQUEST_TIM3_CH1)
#define DEF_TIM_DMA__BTCH_TIM3_CH2      DEF_TIM_DMA_FULL(DMA_REQUEST_TIM3_CH2)
#define DEF_TIM_DMA__BTCH_TIM3_CH3      DEF_TIM_DMA_FULL(DMA_REQUEST_TIM3_CH3)
#define DEF_TIM_DMA__BTCH_TIM3_CH4      DEF_TIM_DMA_FULL(DMA_REQUEST_TIM3_CH4)
#define DEF_TIM_DMA__BTCH_TIM4_CH1      DEF_TIM_DMA_FULL(DMA_REQUEST_TIM4_CH1)
#define DEF_TIM_DMA__BTCH_TIM4_CH2      DEF_TIM_DMA_FULL(DMA_REQUEST_TIM4_CH2)
#define DEF_TIM_DMA__BTCH_TIM4_CH3      DEF_TIM_DMA_FULL(DMA_REQUEST_TIM4_CH3)
#define DEF_TIM_DMA__BTCH_TIM4_CH4      DEF_TIM_DMA_FULL(DMA_REQUEST_TIM4_CH4)
#define DEF_TIM_DMA__BTCH_TIM5_CH1      DEF_TIM_DMA_FULL(DMA_REQUEST_TIM5_CH1)
#define DEF_TIM_DMA__BTCH_TIM5_CH2      DEF_TIM_DMA_FULL(DMA_REQUEST_TIM5_CH2)
#define DEF_TIM_DMA__BTCH_TIM5_CH3      DEF_TIM_DMA_FULL(DMA_REQUEST_TIM5_CH3)
#define DEF_TIM_DMA__BTCH_TIM5_CH4      DEF_TIM_DMA_FULL(DMA_REQUEST_TIM5_CH4)
#define DEF_TIM_DMA__BTCH_TIM8_CH1      DEF_TIM_DMA_FULL(DMA_REQUEST_TIM8_CH1)
#define DEF_TIM_DMA__BTCH_TIM8_CH2      DEF_TIM_DMA_FULL(DMA_REQUEST_TIM8_CH2)
#define DEF_TIM_DMA__BTCH_TIM8_CH3      DEF_TIM_DMA_FULL(DMA_REQUEST_TIM8_CH3)
#define DEF_TIM_DMA__BTCH_TIM8_CH4      DEF_TIM_DMA_FULL(DMA_REQUEST_TIM8_CH4)
#define DEF_TIM_DMA__BTCH_TIM15_CH1     DEF_TIM_DMA_FULL(DMA_REQUEST_TIM15_CH1)
#define DEF_TIM_DMA__BTCH_TIM16_CH1     DEF_TIM_DMA_FULL(DMA_REQUEST_TIM16_CH1)
#define DEF_TIM_DMA__BTCH_TIM17_CH1     DEF_TIM_DMA_FULL(DMA_REQUEST_TIM17_CH1)
#define DEF_TIM_DMA__BTCH_TIM20_CH1     DEF_TIM_DMA_FULL(DMA_REQUEST_TIM20_CH1)
#define DEF_TIM_DMA__BTCH_TIM20_CH2     DEF_TIM_DMA_FULL(DMA_REQUEST_TIM20_CH2)
#define DEF_TIM_DMA__BTCH_TIM20_CH3     DEF_TIM_DMA_FULL(DMA_REQUEST_TIM20_CH3)
#define DEF_TIM_DMA__BTCH_TIM20_CH4     DEF_TIM_DMA_FULL(DMA_REQUEST_TIM20_CH4)
#define DEF_TIM_DMA__BTCH_TIM15_CH2   NONE

// TIM_UP table
#define DEF_TIM_DMA__BTCH_TIM1_UP       DEF_TIM_DMA_FULL(DMA_REQUEST_TIM1_UP)
#define DEF_TIM_DMA__BTCH_TIM2_UP       DEF_TIM_DMA_FULL(DMA_REQUEST_TIM2_UP)
#define DEF_TIM_DMA__BTCH_TIM3_UP       DEF_TIM_DMA_FULL(DMA_REQUEST_TIM3_UP)
#define DEF_TIM_DMA__BTCH_TIM4_UP       DEF_TIM_DMA_FULL(DMA_REQUEST_TIM4_UP)
#define DEF_TIM_DMA__BTCH_TIM5_UP       DEF_TIM_DMA_FULL(DMA_REQUEST_TIM5_UP)
#define DEF_TIM_DMA__BTCH_TIM6_UP       DEF_TIM_DMA_FULL(DMA_REQUEST_TIM6_UP)
#define DEF_TIM_DMA__BTCH_TIM7_UP       DEF_TIM_DMA_FULL(DMA_REQUEST_TIM7_UP)
#define DEF_TIM_DMA__BTCH_TIM8_UP       DEF_TIM_DMA_FULL(DMA_REQUEST_TIM8_UP)
#define DEF_TIM_DMA__BTCH_TIM15_UP      DEF_TIM_DMA_FULL(DMA_REQUEST_TIM15_UP)
#define DEF_TIM_DMA__BTCH_TIM16_UP      DEF_TIM_DMA_FULL(DMA_REQUEST_TIM16_UP)
#define DEF_TIM_DMA__BTCH_TIM17_UP      DEF_TIM_DMA_FULL(DMA_REQUEST_TIM17_UP)
#define DEF_TIM_DMA__BTCH_TIM20_UP      DEF_TIM_DMA_FULL(DMA_REQUEST_TIM20_UP)

// AF table


//PORTA
#define DEF_TIM_AF__PA0__TCH_TIM2_CH1     D(1, 2)
#define DEF_TIM_AF__PA1__TCH_TIM2_CH2     D(1, 2)
#define DEF_TIM_AF__PA2__TCH_TIM2_CH3     D(1, 2)
#define DEF_TIM_AF__PA3__TCH_TIM2_CH4     D(1, 2)
#define DEF_TIM_AF__PA5__TCH_TIM2_CH1     D(1, 2)
#define DEF_TIM_AF__PA6__TCH_TIM16_CH1    D(1, 16)
#define DEF_TIM_AF__PA7__TCH_TIM17_CH1    D(1, 17)
#define DEF_TIM_AF__PA12__TCH_TIM16_CH1   D(1, 16)
#define DEF_TIM_AF__PA13__TCH_TIM16_CH1N  D(1, 16)
#define DEF_TIM_AF__PA15__TCH_TIM2_CH1    D(1, 2)

#define DEF_TIM_AF__PA0__TCH_TIM5_CH1     D(2, 5)
#define DEF_TIM_AF__PA1__TCH_TIM5_CH2     D(2, 5)
#define DEF_TIM_AF__PA2__TCH_TIM5_CH3     D(2, 5)
#define DEF_TIM_AF__PA3__TCH_TIM5_CH4     D(2, 5)
#define DEF_TIM_AF__PA4__TCH_TIM3_CH2     D(2, 3)
#define DEF_TIM_AF__PA6__TCH_TIM3_CH1     D(2, 3)
#define DEF_TIM_AF__PA7__TCH_TIM3_CH2     D(2, 3)
#define DEF_TIM_AF__PA15__TCH_TIM8_CH1     D(2, 8)

#define DEF_TIM_AF__PA7__TCH_TIM8_CH1N    D(4, 8)

#define DEF_TIM_AF__PA14__TCH_TIM8_CH2     D(5, 8)

#define DEF_TIM_AF__PA7__TCH_TIM1_CH1N    D(6, 1)
#define DEF_TIM_AF__PA8__TCH_TIM1_CH1     D(6, 1)
#define DEF_TIM_AF__PA9__TCH_TIM1_CH2     D(6, 1)
#define DEF_TIM_AF__PA10__TCH_TIM1_CH3    D(6, 1)
#define DEF_TIM_AF__PA11__TCH_TIM1_CH1N   D(6, 1)
#define DEF_TIM_AF__PA12__TCH_TIM1_CH2N   D(6, 1)

#define DEF_TIM_AF__PA1__TCH_TIM15_CH1N   D(9, 15)
#define DEF_TIM_AF__PA2__TCH_TIM15_CH1    D(9, 15)
#define DEF_TIM_AF__PA3__TCH_TIM15_CH2    D(9, 15)

#define DEF_TIM_AF__PA9__TCH_TIM2_CH3     D(10, 2)
#define DEF_TIM_AF__PA10__TCH_TIM2_CH4    D(10, 2)
#define DEF_TIM_AF__PA11__TCH_TIM4_CH1    D(10, 4)
#define DEF_TIM_AF__PA12__TCH_TIM4_CH2    D(10, 4)
#define DEF_TIM_AF__PA13__TCH_TIM4_CH3    D(10, 4)

#define DEF_TIM_AF__PA11__TCH_TIM1_CH4    D(11, 1)

//PORTB
#define DEF_TIM_AF__PB3__TCH_TIM2_CH2     D(1, 2)
#define DEF_TIM_AF__PB4__TCH_TIM16_CH1    D(1, 16)
#define DEF_TIM_AF__PB6__TCH_TIM16_CH1N   D(1, 16)
#define DEF_TIM_AF__PB7__TCH_TIM17_CH1N   D(1, 17)
#define DEF_TIM_AF__PB8__TCH_TIM16_CH1    D(1, 16)
#define DEF_TIM_AF__PB9__TCH_TIM17_CH1    D(1, 17)
#define DEF_TIM_AF__PB10__TCH_TIM2_CH3    D(1, 2)
#define DEF_TIM_AF__PB11__TCH_TIM2_CH4    D(1, 2)
#define DEF_TIM_AF__PB14__TCH_TIM15_CH1   D(1, 15)
#define DEF_TIM_AF__PB15__TCH_TIM15_CH2   D(1, 15)

#define DEF_TIM_AF__PB0__TCH_TIM3_CH3     D(2, 3)
#define DEF_TIM_AF__PB1__TCH_TIM3_CH4     D(2, 3)
#define DEF_TIM_AF__PB2__TCH_TIM5_CH1     D(2, 5)
#define DEF_TIM_AF__PB4__TCH_TIM3_CH1     D(2, 3)
#define DEF_TIM_AF__PB5__TCH_TIM3_CH2     D(2, 3)
#define DEF_TIM_AF__PB6__TCH_TIM4_CH1     D(2, 4)
#define DEF_TIM_AF__PB7__TCH_TIM4_CH2     D(2, 4)
#define DEF_TIM_AF__PB8__TCH_TIM4_CH3     D(2, 4)
#define DEF_TIM_AF__PB9__TCH_TIM4_CH4     D(2, 4)
#define DEF_TIM_AF__PB15__TCH_TIM15_CH1N  D(2, 15)

#define DEF_TIM_AF__PB2__TCH_TIM20_CH1    D(3, 20)
#define DEF_TIM_AF__PB5__TCH_TIM8_CH3N    D(3, 8)

#define DEF_TIM_AF__PB0__TCH_TIM8_CH2N    D(4, 8)
#define DEF_TIM_AF__PB1__TCH_TIM8_CH3N    D(4, 8)
#define DEF_TIM_AF__PB3__TCH_TIM8_CH1N    D(4, 8)
#define DEF_TIM_AF__PB4__TCH_TIM8_CH2N    D(4, 8)
#define DEF_TIM_AF__PB15__TCH_TIM1_CH3N   D(4, 1)

#define DEF_TIM_AF__PB6__TCH_TIM8_CH1     D(5, 8)

#define DEF_TIM_AF__PB0__TCH_TIM1_CH2N    D(6, 1)
#define DEF_TIM_AF__PB1__TCH_TIM1_CH3N    D(6, 1)
#define DEF_TIM_AF__PB13__TCH_TIM1_CH1N   D(6, 1)
#define DEF_TIM_AF__PB14__TCH_TIM1_CH2N   D(6, 1)

#define DEF_TIM_AF__PB5__TCH_TIM17_CH1    D(10, 17)
#define DEF_TIM_AF__PB7__TCH_TIM3_CH4     D(10, 3)
#define DEF_TIM_AF__PB8__TCH_TIM8_CH2     D(10, 8)
#define DEF_TIM_AF__PB9__TCH_TIM8_CH3     D(10, 8)

#define DEF_TIM_AF__PB9__TCH_TIM1_CH3N    D(12, 1)

//PORTC
#define DEF_TIM_AF__PC12__TCH_TIM5_CH2    D(1, 5)

#define DEF_TIM_AF__PC0__TCH_TIM1_CH1     D(2, 1)
#define DEF_TIM_AF__PC1__TCH_TIM1_CH2     D(2, 1)
#define DEF_TIM_AF__PC2__TCH_TIM1_CH3     D(2, 1)
#define DEF_TIM_AF__PC3__TCH_TIM1_CH4     D(2, 1)
#define DEF_TIM_AF__PC6__TCH_TIM3_CH1     D(2, 3)
#define DEF_TIM_AF__PC7__TCH_TIM3_CH2     D(2, 3)
#define DEF_TIM_AF__PC8__TCH_TIM3_CH3     D(2, 3)
#define DEF_TIM_AF__PC9__TCH_TIM3_CH4     D(2, 3)

#define DEF_TIM_AF__PC6__TCH_TIM8_CH1     D(4, 8)
#define DEF_TIM_AF__PC7__TCH_TIM8_CH2     D(4, 8)
#define DEF_TIM_AF__PC8__TCH_TIM8_CH3     D(4, 8)
#define DEF_TIM_AF__PC9__TCH_TIM8_CH4     D(4, 8)
#define DEF_TIM_AF__PC10__TCH_TIM8_CH1N   D(4, 8)
#define DEF_TIM_AF__PC11__TCH_TIM8_CH2N   D(4, 8)
#define DEF_TIM_AF__PC12__TCH_TIM8_CH3N   D(4, 8)
#define DEF_TIM_AF__PC13__TCH_TIM1_CH1N   D(4, 1)

#define DEF_TIM_AF__PC2__TCH_TIM20_CH2    D(6, 20)
#define DEF_TIM_AF__PC5__TCH_TIM1_CH4N    D(6, 1)
#define DEF_TIM_AF__PC8__TCH_TIM20_CH3    D(6, 20)
#define DEF_TIM_AF__PC13__TCH_TIM8_CH4N   D(6, 8)

