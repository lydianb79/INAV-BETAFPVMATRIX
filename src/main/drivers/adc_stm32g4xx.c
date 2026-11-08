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
 * STM32G47x ADC driver (ADC1 and ADC2, continuous scan with circular DMA).
 *
 * DMA2 channel 1 serves ADC1 and DMA2 channel 2 serves ADC2, so the timer/DShot DMA
 * assignments (DMA1 channels) never collide with them.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "platform.h"

#include "build/debug.h"

#include "drivers/time.h"

#include "drivers/io.h"
#include "io_impl.h"
#include "rcc.h"
#include "dma.h"

#include "drivers/sensor.h"
#include "drivers/accgyro/accgyro.h"

#include "adc.h"
#include "adc_impl.h"

static adcDevice_t adcHardware[ADCDEV_COUNT] = {
    {
        .ADCx = ADC1,
        .rccADC = RCC_AHB2(ADC12),
        .rccDMA = RCC_AHB1(DMA2),
        .DMAy_Channelx = DMA2_Channel1,
        .channel = DMA_REQUEST_ADC1,
        .enabled = false,
        .usedChannelCount = 0
    },
    {
        .ADCx = ADC2,
        .rccADC = RCC_AHB2(ADC12),
        .rccDMA = RCC_AHB1(DMA2),
        .DMAy_Channelx = DMA2_Channel2,
        .channel = DMA_REQUEST_ADC2,
        .enabled = false,
        .usedChannelCount = 0
    },
};

adcDevice_t adcDevice[ADCDEV_COUNT];

/*
 * Pin to channel map, DS12288 Table 12.
 * Pins wired to ADC1 use their ADC1 channel number. Pins that only reach ADC2
 * (PA4..PA7, PC4, PC5, PB2, PB15) use their ADC2 channel number; a target using
 * one of those must also define ADC_CHANNEL_x_INSTANCE ADC2.
 */
const adcTagMap_t adcTagMap[] = {
    // ADC1 (PA0/PA1/PC0..PC3 are shared with ADC2 on the same channel number)
    { DEFIO_TAG_E__PA0,  ADC_CHANNEL_1  },
    { DEFIO_TAG_E__PA1,  ADC_CHANNEL_2  },
    { DEFIO_TAG_E__PA2,  ADC_CHANNEL_3  },
    { DEFIO_TAG_E__PA3,  ADC_CHANNEL_4  },
    { DEFIO_TAG_E__PB14, ADC_CHANNEL_5  },
    { DEFIO_TAG_E__PC0,  ADC_CHANNEL_6  },
    { DEFIO_TAG_E__PC1,  ADC_CHANNEL_7  },
    { DEFIO_TAG_E__PC2,  ADC_CHANNEL_8  },
    { DEFIO_TAG_E__PC3,  ADC_CHANNEL_9  },
    { DEFIO_TAG_E__PF0,  ADC_CHANNEL_10 },
    { DEFIO_TAG_E__PB12, ADC_CHANNEL_11 },
    { DEFIO_TAG_E__PB1,  ADC_CHANNEL_12 },
    { DEFIO_TAG_E__PB11, ADC_CHANNEL_14 },
    { DEFIO_TAG_E__PB0,  ADC_CHANNEL_15 },
    // ADC2 only
    { DEFIO_TAG_E__PA6,  ADC_CHANNEL_3  },
    { DEFIO_TAG_E__PA7,  ADC_CHANNEL_4  },
    { DEFIO_TAG_E__PC4,  ADC_CHANNEL_5  },
    { DEFIO_TAG_E__PF1,  ADC_CHANNEL_10 },
    { DEFIO_TAG_E__PC5,  ADC_CHANNEL_11 },
    { DEFIO_TAG_E__PB2,  ADC_CHANNEL_12 },
    { DEFIO_TAG_E__PA5,  ADC_CHANNEL_13 },
    { DEFIO_TAG_E__PB15, ADC_CHANNEL_15 },
    { DEFIO_TAG_E__PA4,  ADC_CHANNEL_17 },
};

// Translate rank number x to ADC_REGULAR_RANK_x (Note that array index is 0-origin)
static const uint32_t adcRegularRankMap[] = {
    ADC_REGULAR_RANK_1,
    ADC_REGULAR_RANK_2,
    ADC_REGULAR_RANK_3,
    ADC_REGULAR_RANK_4,
    ADC_REGULAR_RANK_5,
    ADC_REGULAR_RANK_6,
    ADC_REGULAR_RANK_7,
    ADC_REGULAR_RANK_8,
    ADC_REGULAR_RANK_9,
    ADC_REGULAR_RANK_10,
    ADC_REGULAR_RANK_11,
    ADC_REGULAR_RANK_12,
    ADC_REGULAR_RANK_13,
    ADC_REGULAR_RANK_14,
    ADC_REGULAR_RANK_15,
    ADC_REGULAR_RANK_16,
};

ADCDevice adcDeviceByInstance(ADC_TypeDef *instance)
{
    if (instance == ADC1)
        return ADCDEV_1;

    if (instance == ADC2)
        return ADCDEV_2;

    return ADCINVALID;
}

static void adcInstanceInit(ADCDevice adcDevice)
{
    adcDevice_t * adc = &adcHardware[adcDevice];

    RCC_ClockCmd(adc->rccDMA, ENABLE);
    RCC_ClockCmd(RCC_AHB1(DMAMUX1), ENABLE);
    RCC_ClockCmd(adc->rccADC, ENABLE);

    adc->ADCHandle.Instance = adc->ADCx;

    // HCLK / 4 = 42 MHz (max ADC clock is 60 MHz)
    adc->ADCHandle.Init.ClockPrescaler        = ADC_CLOCK_SYNC_PCLK_DIV4;
    adc->ADCHandle.Init.Resolution            = ADC_RESOLUTION_12B;
    adc->ADCHandle.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
    adc->ADCHandle.Init.GainCompensation      = 0;
    adc->ADCHandle.Init.ScanConvMode          = adc->usedChannelCount > 1 ? ADC_SCAN_ENABLE : ADC_SCAN_DISABLE;
    adc->ADCHandle.Init.EOCSelection          = ADC_EOC_SINGLE_CONV;
    adc->ADCHandle.Init.LowPowerAutoWait      = DISABLE;
    adc->ADCHandle.Init.ContinuousConvMode    = ENABLE;
    adc->ADCHandle.Init.NbrOfConversion       = adc->usedChannelCount;
    adc->ADCHandle.Init.DiscontinuousConvMode = DISABLE;
    adc->ADCHandle.Init.NbrOfDiscConversion   = 1;
    adc->ADCHandle.Init.ExternalTrigConv      = ADC_SOFTWARE_START;
    adc->ADCHandle.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
    adc->ADCHandle.Init.SamplingMode          = ADC_SAMPLING_MODE_NORMAL;
    adc->ADCHandle.Init.DMAContinuousRequests = ENABLE;   // circular DMA
    adc->ADCHandle.Init.Overrun               = ADC_OVR_DATA_OVERWRITTEN;
    adc->ADCHandle.Init.OversamplingMode      = DISABLE;

    if (HAL_ADC_Init(&adc->ADCHandle) != HAL_OK) {
        return;
    }

    if (HAL_ADCEx_Calibration_Start(&adc->ADCHandle, ADC_SINGLE_ENDED) != HAL_OK) {
        return;
    }

    adc->DmaHandle.Instance = adc->DMAy_Channelx;
    adc->DmaHandle.Init.Request = adc->channel;
    adc->DmaHandle.Init.Direction = DMA_PERIPH_TO_MEMORY;
    adc->DmaHandle.Init.PeriphInc = DMA_PINC_DISABLE;
    adc->DmaHandle.Init.MemInc = ((adc->usedChannelCount > 1) || (ADC_AVERAGE_N_SAMPLES > 1)) ? DMA_MINC_ENABLE : DMA_MINC_DISABLE;
    adc->DmaHandle.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    adc->DmaHandle.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    adc->DmaHandle.Init.Mode = DMA_CIRCULAR;
    adc->DmaHandle.Init.Priority = DMA_PRIORITY_HIGH;

    if (HAL_DMA_Init(&adc->DmaHandle) != HAL_OK) {
        return;
    }

    __HAL_LINKDMA(&adc->ADCHandle, DMA_Handle, adc->DmaHandle);

    uint8_t rank = 0;
    for (int i = ADC_CHN_1; i < ADC_CHN_COUNT; i++) {
        if (!adcConfig[i].enabled || adcConfig[i].adcDevice != adcDevice) {
            continue;
        }

        ADC_ChannelConfTypeDef sConfig;
        memset(&sConfig, 0, sizeof(sConfig));
        sConfig.Channel      = adcConfig[i].adcChannel;
        sConfig.Rank         = adcRegularRankMap[rank++];
        sConfig.SamplingTime = adcConfig[i].sampleTime;
        sConfig.SingleDiff   = ADC_SINGLE_ENDED;
        sConfig.OffsetNumber = ADC_OFFSET_NONE;
        sConfig.Offset       = 0;

        if (HAL_ADC_ConfigChannel(&adc->ADCHandle, &sConfig) != HAL_OK) {
            return;
        }
    }

    // The ADC reads into the buffer continuously; the HAL DMA interrupts are not needed
    if (HAL_ADC_Start_DMA(&adc->ADCHandle, (uint32_t*)&adcValues[adcDevice], adc->usedChannelCount * ADC_AVERAGE_N_SAMPLES) != HAL_OK) {
        return;
    }
    __HAL_DMA_DISABLE_IT(&adc->DmaHandle, DMA_IT_TC | DMA_IT_HT | DMA_IT_TE);
}

void adcHardwareInit(drv_adc_config_t *init)
{
    UNUSED(init);
    int configuredAdcChannels = 0;

    for (int i = ADC_CHN_1; i < ADC_CHN_COUNT; i++) {
        if (!adcConfig[i].tag)
            continue;

        adcDevice_t * adc = &adcHardware[adcConfig[i].adcDevice];

        IOInit(IOGetByTag(adcConfig[i].tag), OWNER_ADC, RESOURCE_ADC_CH1 + (i - ADC_CHN_1), 0);
        IOConfigGPIO(IOGetByTag(adcConfig[i].tag), IO_CONFIG(GPIO_MODE_ANALOG, 0, GPIO_NOPULL));

        adcConfig[i].adcChannel = adcChannelByTag(adcConfig[i].tag);
        adcConfig[i].dmaIndex = adc->usedChannelCount++;
        adcConfig[i].sampleTime = ADC_SAMPLETIME_640CYCLES_5;
        adcConfig[i].enabled = true;

        adc->enabled = true;
        configuredAdcChannels++;
    }

    if (configuredAdcChannels == 0)
        return;

    for (int i = 0; i < ADCDEV_COUNT; i++) {
        if (adcHardware[i].enabled) {
            adcInstanceInit(i);
        }
    }
}
