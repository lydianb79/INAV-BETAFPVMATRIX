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

#include <stdbool.h>
#include <stdint.h>

#include "platform.h"

#include "common/utils.h"

#include "drivers/io.h"
#include "drivers/rcc.h"
#include "drivers/time.h"
#include "drivers/nvic.h"
#include "drivers/timer.h"
#include "drivers/timer_impl.h"

#include "stm32g4xx.h"

// Interrupt layout on G47x:
//  TIM1:  TIM1_CC_IRQn (CC), TIM1_UP_TIM16_IRQn, TIM1_BRK_TIM15_IRQn, TIM1_TRG_COM_TIM17_IRQn
//  TIM8:  TIM8_CC_IRQn (CC), TIM8_UP_IRQn (update)
//  TIM20: TIM20_CC_IRQn (CC), TIM20_UP_IRQn (update)
//  TIM15/16/17 share their vectors with TIM1 BRK/UP/TRG_COM
const timerDef_t timerDefinitions[HARDWARE_TIMER_DEFINITION_COUNT] = {
    [TIMER_INDEX(1)]  = { .tim = TIM1,  .rcc = RCC_APB2(TIM1),   .irq = TIM1_CC_IRQn,  .secondIrq = TIM1_UP_TIM16_IRQn },
    [TIMER_INDEX(2)]  = { .tim = TIM2,  .rcc = RCC_APB11(TIM2),  .irq = TIM2_IRQn },
    [TIMER_INDEX(3)]  = { .tim = TIM3,  .rcc = RCC_APB11(TIM3),  .irq = TIM3_IRQn },
    [TIMER_INDEX(4)]  = { .tim = TIM4,  .rcc = RCC_APB11(TIM4),  .irq = TIM4_IRQn },
    [TIMER_INDEX(5)]  = { .tim = TIM5,  .rcc = RCC_APB11(TIM5),  .irq = TIM5_IRQn },
    [TIMER_INDEX(6)]  = { .tim = TIM6,  .rcc = RCC_APB11(TIM6),  .irq = 0 },
    [TIMER_INDEX(7)]  = { .tim = TIM7,  .rcc = RCC_APB11(TIM7),  .irq = 0 },
    [TIMER_INDEX(8)]  = { .tim = TIM8,  .rcc = RCC_APB2(TIM8),   .irq = TIM8_CC_IRQn,  .secondIrq = TIM8_UP_IRQn },
    [TIMER_INDEX(15)] = { .tim = TIM15, .rcc = RCC_APB2(TIM15),  .irq = TIM1_BRK_TIM15_IRQn },
    [TIMER_INDEX(16)] = { .tim = TIM16, .rcc = RCC_APB2(TIM16),  .irq = TIM1_UP_TIM16_IRQn },
    [TIMER_INDEX(17)] = { .tim = TIM17, .rcc = RCC_APB2(TIM17),  .irq = TIM1_TRG_COM_TIM17_IRQn },
    [TIMER_INDEX(20)] = { .tim = TIM20, .rcc = RCC_APB2(TIM20),  .irq = TIM20_CC_IRQn, .secondIrq = TIM20_UP_IRQn },
};

// The timer table must cover every slot of timerCtx[] - see TIMER_INDEX() in timer.h
STATIC_ASSERT(TIMER_INDEX(20) == HARDWARE_TIMER_DEFINITION_COUNT - 1, timer_definition_table_size_mismatch);

uint32_t timerClock(TIM_TypeDef *tim)
{
    /*
     * RM0440 6.2.13 Timer clock: if the APB prescaler is 1 the timer kernel clock equals
     * the APB clock, otherwise it is twice the APB clock.
     */
    uint32_t pclk;

    if (tim == TIM1 || tim == TIM8 || tim == TIM15 || tim == TIM16 || tim == TIM17 || tim == TIM20) {
        pclk = HAL_RCC_GetPCLK2Freq();
        if (READ_BIT(RCC->CFGR, RCC_CFGR_PPRE2) & RCC_CFGR_PPRE2_2) {
            pclk *= 2;
        }
    } else {
        pclk = HAL_RCC_GetPCLK1Freq();
        if (READ_BIT(RCC->CFGR, RCC_CFGR_PPRE1) & RCC_CFGR_PPRE1_2) {
            pclk *= 2;
        }
    }

    return pclk;
}

_TIM_IRQ_HANDLER(TIM1_CC_IRQHandler, 1);
_TIM_IRQ_HANDLER(TIM2_IRQHandler, 2);
_TIM_IRQ_HANDLER(TIM3_IRQHandler, 3);
_TIM_IRQ_HANDLER(TIM4_IRQHandler, 4);
_TIM_IRQ_HANDLER(TIM5_IRQHandler, 5);
_TIM_IRQ_HANDLER(TIM8_CC_IRQHandler, 8);
_TIM_IRQ_HANDLER(TIM8_UP_IRQHandler, 8);
_TIM_IRQ_HANDLER(TIM1_BRK_TIM15_IRQHandler, 15);
_TIM_IRQ_HANDLER2(TIM1_UP_TIM16_IRQHandler, 1, 16);   // TIM1 update + TIM16
_TIM_IRQ_HANDLER(TIM1_TRG_COM_TIM17_IRQHandler, 17);
_TIM_IRQ_HANDLER(TIM20_CC_IRQHandler, 20);
_TIM_IRQ_HANDLER(TIM20_UP_IRQHandler, 20);
