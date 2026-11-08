/*
 * This file is part of INAV.
 *
 * INAV is free software. You can redistribute this software
 * and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * INAV is distributed in the hope that they will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>

#include "platform.h"

#include "drivers/bus.h"
#include "drivers/io.h"
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"
#include "drivers/sensor.h"

/*
 * Timers resolved from the Betaflight BETAFPVG473 TIMER_PIN_MAPPING
 * (option index = n-th entry for that pin in the G4 full timer table):
 *   PB2  opt 1 -> TIM5_CH1    LED strip
 *   PB0  opt 1 -> TIM3_CH3    motor 1
 *   PB1  opt 1 -> TIM3_CH4    motor 2
 *   PB6  opt 3 -> TIM8_CH1    motor 3
 *   PC13 opt 2 -> TIM8_CH4N   motor 4
 * DMA: last argument = DMA1 channel - 1 (DMAMUX routes the timer requests)
 */
timerHardware_t timerHardware[] = {
    DEF_TIM(TIM3, CH3,  PB0,  TIM_USE_MOTOR, 0, 0),   // M1, DMA1_CH1
    DEF_TIM(TIM3, CH4,  PB1,  TIM_USE_MOTOR, 0, 1),   // M2, DMA1_CH2
    DEF_TIM(TIM8, CH1,  PB6,  TIM_USE_MOTOR, 0, 2),   // M3, DMA1_CH3
    DEF_TIM(TIM8, CH4N, PC13, TIM_USE_MOTOR, 0, 3),   // M4, DMA1_CH4

    DEF_TIM(TIM5, CH1,  PB2,  TIM_USE_LED,   0, 4),   // LED strip, DMA1_CH5
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
