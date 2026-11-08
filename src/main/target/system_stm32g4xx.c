/**
  ******************************************************************************
  * @file    system_stm32g4xx.c
  * @brief   CMSIS Cortex-M4 Device Peripheral Access Layer System Source File
  *          for STM32G47x, adapted for INAV.
  *
  *          Clock tree set up here:
  *            SYSCLK = PLL R = 168 MHz (VCO 336 MHz, VCO input 4 MHz)
  *            PLL source = HSE (HSE_VALUE, any multiple of 4 MHz up to 48 MHz)
  *                         or HSI16 when the target defines USE_HSI_CLOCK
  *                         or when the HSE crystal fails to start.
  *            HCLK = PCLK1 = PCLK2 = 168 MHz, so timer clocks are 168 MHz as well
  *            USB  = HSI48 trimmed by the CRS from USB SOF
  *            ADC12/345 = SYSCLK, U(S)ARTs and I2Cs = PCLK
  ******************************************************************************
  * Portions Copyright (c) 2019 STMicroelectronics. All rights reserved.
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  ******************************************************************************
  */

#include <string.h>

#include "platform.h"
#include "drivers/system.h"
#include "drivers/persistent.h"

#if !defined  (HSE_VALUE)
#define HSE_VALUE     8000000U  /*!< Value of the External oscillator in Hz */
#endif

#if !defined  (HSI_VALUE)
#define HSI_VALUE    16000000U  /*!< Value of the Internal oscillator in Hz */
#endif

#define SYSCLK_TARGET_MHZ   168U
#define PLL_VCO_INPUT_MHZ   4U

uint32_t SystemCoreClock = HSI_VALUE;

const uint8_t AHBPrescTable[16] = {0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 1U, 2U, 3U, 4U, 6U, 7U, 8U, 9U};
const uint8_t APBPrescTable[8] =  {0U, 0U, 0U, 0U, 1U, 2U, 3U, 4U};

// Reported to the configurator through MSP; 0 means "running from HSI"
uint32_t hse_value = HSE_VALUE;

void Error_Handler(void)
{
    // A clock that cannot be set up is unrecoverable; reset and try again
    __disable_irq();
    NVIC_SystemReset();
}

static bool SystemClock_ConfigPLL(bool useHse)
{
    RCC_OscInitTypeDef RCC_OscInitStruct;
    memset(&RCC_OscInitStruct, 0, sizeof(RCC_OscInitStruct));

    const uint32_t pllInputMhz = useHse ? (HSE_VALUE / 1000000U) : (HSI_VALUE / 1000000U);

    // HSE is always in the mask so the HSI fallback also switches off a crystal that failed to start
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_HSI48 | RCC_OSCILLATORTYPE_LSI
                                     | RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = useHse ? RCC_HSE_ON : RCC_HSE_OFF;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.LSIState = RCC_LSI_ON;
    RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;

    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = useHse ? RCC_PLLSOURCE_HSE : RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = pllInputMhz / PLL_VCO_INPUT_MHZ;               // VCO input = 4 MHz
    RCC_OscInitStruct.PLL.PLLN = (SYSCLK_TARGET_MHZ * 2U) / PLL_VCO_INPUT_MHZ;  // VCO = 336 MHz
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
    RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;                                 // SYSCLK = 168 MHz

    return HAL_RCC_OscConfig(&RCC_OscInitStruct) == HAL_OK;
}

void SystemClock_Config(void)
{
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_PeriphCLKInitTypeDef PeriphClkInit;
    RCC_CRSInitTypeDef crsInit;

    memset(&RCC_ClkInitStruct, 0, sizeof(RCC_ClkInitStruct));
    memset(&PeriphClkInit, 0, sizeof(PeriphClkInit));
    memset(&crsInit, 0, sizeof(crsInit));

    // Range 1 boost mode is required above 150 MHz
    __HAL_RCC_PWR_CLK_ENABLE();
    HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

#if defined(USE_HSI_CLOCK)
    const bool tryHse = false;
#else
    const bool tryHse = (HSE_VALUE % (PLL_VCO_INPUT_MHZ * 1000000U)) == 0;
#endif

    bool pllOk = false;
    if (tryHse) {
        pllOk = SystemClock_ConfigPLL(true);
    }
    if (!pllOk) {
        // No crystal fitted, or it did not start: fall back to the internal 16 MHz RC
        hse_value = 0;
        if (!SystemClock_ConfigPLL(false)) {
            Error_Handler();
        }
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    // 168 MHz in range 1 boost mode needs 4 wait states (RM0440 Table 9)
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK) {
        Error_Handler();
    }

    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1 | RCC_PERIPHCLK_USART2 | RCC_PERIPHCLK_USART3
                                       | RCC_PERIPHCLK_UART4 | RCC_PERIPHCLK_UART5 | RCC_PERIPHCLK_LPUART1
                                       | RCC_PERIPHCLK_I2C1 | RCC_PERIPHCLK_I2C2 | RCC_PERIPHCLK_I2C3 | RCC_PERIPHCLK_I2C4
                                       | RCC_PERIPHCLK_USB | RCC_PERIPHCLK_ADC12 | RCC_PERIPHCLK_ADC345;
    PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
    PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
    PeriphClkInit.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
    PeriphClkInit.Uart4ClockSelection = RCC_UART4CLKSOURCE_PCLK1;
    PeriphClkInit.Uart5ClockSelection = RCC_UART5CLKSOURCE_PCLK1;
    PeriphClkInit.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_PCLK1;
    PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
    PeriphClkInit.I2c2ClockSelection = RCC_I2C2CLKSOURCE_PCLK1;
    PeriphClkInit.I2c3ClockSelection = RCC_I2C3CLKSOURCE_PCLK1;
    PeriphClkInit.I2c4ClockSelection = RCC_I2C4CLKSOURCE_PCLK1;
    PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
    PeriphClkInit.Adc12ClockSelection = RCC_ADC12CLKSOURCE_SYSCLK;
    PeriphClkInit.Adc345ClockSelection = RCC_ADC345CLKSOURCE_SYSCLK;

    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
        Error_Handler();
    }

    // Trim HSI48 from the USB start-of-frame packets so USB works without a crystal
    __HAL_RCC_CRS_CLK_ENABLE();
    crsInit.Prescaler = RCC_CRS_SYNC_DIV1;
    crsInit.Source = RCC_CRS_SYNC_SOURCE_USB;
    crsInit.Polarity = RCC_CRS_SYNC_POLARITY_RISING;
    crsInit.ReloadValue = __HAL_RCC_CRS_RELOADVALUE_CALCULATE(48000000, 1000);
    crsInit.ErrorLimitValue = 34;
    crsInit.HSI48CalibrationValue = 32;
    HAL_RCCEx_CRSConfig(&crsInit);

    // Prefetch, instruction and data caches on
    SET_BIT(FLASH->ACR, FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | FLASH_ACR_DCEN);
}

void SystemInit(void)
{
    initialiseMemorySections();

    // The bootloader request flag lives in the TAMP backup registers, which
    // are only readable with the RTC APB clock on
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_RCC_RTCAPB_CLK_ENABLE();

    // Check for bootloader request BEFORE any clock/peripheral configuration
    checkForBootLoaderRequest();

    // FPU settings
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
    SCB->CPACR |= ((3UL << (10*2)) | (3UL << (11*2)));  // Set CP10 and CP11 Full Access
#endif

    // Vector table is at the start of flash
    extern uint8_t isr_vector_table_base;
    SCB->VTOR = (uint32_t)&isr_vector_table_base;

#ifdef USE_HAL_DRIVER
    HAL_Init();
#endif

    SystemClock_Config();
    SystemCoreClockUpdate();
}

void SystemCoreClockUpdate(void)
{
    SystemCoreClock = HAL_RCC_GetSysClockFreq() >> AHBPrescTable[(RCC->CFGR & RCC_CFGR_HPRE) >> RCC_CFGR_HPRE_Pos];
}
