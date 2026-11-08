/**
  ******************************************************************************
  * @file    system_stm32g4xx.h
  * @brief   CMSIS Cortex-M4 Device System Source File for STM32G4xx devices.
  *          Adapted for INAV.
  ******************************************************************************
  * Copyright (c) 2019 STMicroelectronics. All rights reserved.
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  ******************************************************************************
  */

#ifndef SYSTEM_STM32G4XX_H
#define SYSTEM_STM32G4XX_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>

extern uint32_t SystemCoreClock;            /*!< System Clock Frequency (Core Clock) */

extern const uint8_t  AHBPrescTable[16];    /*!< AHB prescalers table values */
extern const uint8_t  APBPrescTable[8];     /*!< APB prescalers table values */

extern void SystemInit(void);
extern void SystemCoreClockUpdate(void);

#ifdef __cplusplus
}
#endif

#endif /* SYSTEM_STM32G4XX_H */
