#pragma once

#if defined(STM32H7) || defined(STM32G4)
typedef uint16_t rccPeriphTag_t;
#else
typedef uint8_t rccPeriphTag_t;
#endif
