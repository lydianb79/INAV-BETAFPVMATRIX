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
#include <string.h>

#include <platform.h>

#include "common/utils.h"
#include "drivers/nvic.h"
#include "drivers/dma.h"
#include "drivers/rcc.h"

/*
 * DMA descriptors: DMA1/DMA2, channels 1..8, 4 flag bits per channel.
 */
static dmaChannelDescriptor_t dmaDescriptors[] = {
    [0]  = DEFINE_DMA_CHANNEL(1, 1,  0),
    [1]  = DEFINE_DMA_CHANNEL(1, 2,  4),
    [2]  = DEFINE_DMA_CHANNEL(1, 3,  8),
    [3]  = DEFINE_DMA_CHANNEL(1, 4, 12),
    [4]  = DEFINE_DMA_CHANNEL(1, 5, 16),
    [5]  = DEFINE_DMA_CHANNEL(1, 6, 20),
    [6]  = DEFINE_DMA_CHANNEL(1, 7, 24),
    [7]  = DEFINE_DMA_CHANNEL(1, 8, 28),

    [8]  = DEFINE_DMA_CHANNEL(2, 1,  0),
    [9]  = DEFINE_DMA_CHANNEL(2, 2,  4),
    [10] = DEFINE_DMA_CHANNEL(2, 3,  8),
    [11] = DEFINE_DMA_CHANNEL(2, 4, 12),
    [12] = DEFINE_DMA_CHANNEL(2, 5, 16),
    [13] = DEFINE_DMA_CHANNEL(2, 6, 20),
    [14] = DEFINE_DMA_CHANNEL(2, 7, 24),
    [15] = DEFINE_DMA_CHANNEL(2, 8, 28),
};

/*
 * DMA IRQ Handlers
 */
DEFINE_DMA_IRQ_HANDLER(1, 1, 0)
DEFINE_DMA_IRQ_HANDLER(1, 2, 1)
DEFINE_DMA_IRQ_HANDLER(1, 3, 2)
DEFINE_DMA_IRQ_HANDLER(1, 4, 3)
DEFINE_DMA_IRQ_HANDLER(1, 5, 4)
DEFINE_DMA_IRQ_HANDLER(1, 6, 5)
DEFINE_DMA_IRQ_HANDLER(1, 7, 6)
DEFINE_DMA_IRQ_HANDLER(1, 8, 7)
DEFINE_DMA_IRQ_HANDLER(2, 1, 8)
DEFINE_DMA_IRQ_HANDLER(2, 2, 9)
DEFINE_DMA_IRQ_HANDLER(2, 3, 10)
DEFINE_DMA_IRQ_HANDLER(2, 4, 11)
DEFINE_DMA_IRQ_HANDLER(2, 5, 12)
DEFINE_DMA_IRQ_HANDLER(2, 6, 13)
DEFINE_DMA_IRQ_HANDLER(2, 7, 14)
DEFINE_DMA_IRQ_HANDLER(2, 8, 15)

DMA_t dmaGetByTag(dmaTag_t tag)
{
    for (unsigned i = 0; i < ARRAYLEN(dmaDescriptors); i++) {
        if (DMATAG_GET_DMA(dmaDescriptors[i].tag) == DMATAG_GET_DMA(tag) && DMATAG_GET_STREAM(dmaDescriptors[i].tag) == DMATAG_GET_STREAM(tag)) {
            return (DMA_t)&dmaDescriptors[i];
        }
    }

    return (DMA_t) NULL;
}

void dmaEnableClock(DMA_t dma)
{
    if (dma->dma == DMA1) {
        RCC_ClockCmd(RCC_AHB1(DMA1), ENABLE);
    }
    else {
        RCC_ClockCmd(RCC_AHB1(DMA2), ENABLE);
    }

    // G4 has a separate clock enable for the DMAMUX that routes requests to channels
    RCC_ClockCmd(RCC_AHB1(DMAMUX1), ENABLE);
}

resourceOwner_e dmaGetOwner(DMA_t dma)
{
    return dma->owner;
}

void dmaInit(DMA_t dma, resourceOwner_e owner, uint8_t resourceIndex)
{
    dmaEnableClock(dma);
    dma->owner = owner;
    dma->resourceIndex = resourceIndex;
}

void dmaSetHandler(DMA_t dma, dmaCallbackHandlerFuncPtr callback, uint32_t priority, uint32_t userParam)
{
    dmaEnableClock(dma);

    dma->irqHandlerCallback = callback;
    dma->userParam = userParam;

    HAL_NVIC_SetPriority(dma->irqNumber, priority, 0);
    HAL_NVIC_EnableIRQ(dma->irqNumber);
}

DMA_t dmaGetByRef(const DMA_Channel_TypeDef* ref)
{
    for (unsigned i = 0; i < ARRAYLEN(dmaDescriptors); i++) {
        if (ref == dmaDescriptors[i].ref) {
            return &dmaDescriptors[i];
        }
    }

    return NULL;
}
