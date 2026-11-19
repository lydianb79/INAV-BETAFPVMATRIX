# Adding an STM32G4 flight controller to this INAV tree

This tree is upstream INAV plus STM32G4 support (see `docs/MATRIX1S_STM32G4_PORT.md`).
A new G4 board only needs a target folder: drop it into `src/main/target/` and build.
INAV picks up every folder in `src/main/target/` that contains a `CMakeLists.txt`.

> Targets built this way are **untested until you test them**. Bench-test every new board
> with **propellers removed**.

## Supported MCUs

| MCU | CMake function | Flash / RAM |
|---|---|---|
| STM32G473xE | `target_stm32g473xe(NAME ...)` | 512 KB / 96 KB SRAM + 32 KB CCM |
| STM32G474xE | `target_stm32g474xe(NAME ...)` | 512 KB / 96 KB SRAM + 32 KB CCM |

Other G4 parts (G473xC/xB with less flash, G431, G491, ...) need their own linker script and
`define_target_stm32g4(...)` line in `cmake/stm32g4.cmake`; they are not set up.

## The four files

Copy `src/main/target/BETAFPVMATRIX_4IN1/` (crystal) or `BETAFPVMATRIX_3IN1/` (no crystal)
to `src/main/target/<YOURTARGET>/` and edit:

### `CMakeLists.txt`

```cmake
target_stm32g473xe(YOURTARGET SKIP_RELEASES HSE_MHZ 8)
```

* `HSE_MHZ` = crystal frequency. It must be a multiple of 4 (8, 12, 16, 24 ...) because the clock
  code derives a 4 MHz PLL input from it; any other value makes the firmware run from the internal
  oscillator instead. Omit it for the default of 8.
* If the board has **no crystal** (Betaflight `SYSTEM_HSE_MHZ 0`), leave `HSE_MHZ` out and put
  `#define USE_HSI_CLOCK` in `target.h`. If a crystal is fitted but fails to start, the firmware
  falls back to the internal oscillator automatically.

### `target.h`

Translate the Betaflight `config.h` (from `github.com/betaflight/config`, `configs/<MFR>/<BOARD>/config.h`):

| Betaflight `config.h` | INAV `target.h` |
|---|---|
| `BOARD_NAME` | `USBD_PRODUCT_STRING "YOURTARGET"`, `TARGET_BOARD_IDENTIFIER "XXXX"` (4 chars) |
| `SYSTEM_HSE_MHZ 0` | `#define USE_HSI_CLOCK` |
| `USE_GYRO_SPI_ICM42688P` | `USE_IMU_ICM42605` + `ICM42605_SPI_BUS`, `ICM42605_CS_PIN`, `IMU_ICM42605_ALIGN` (also covers ICM42622P) |
| `USE_ACCGYRO_BMI270` | `USE_IMU_BMI270` + `BMI270_SPI_BUS`, `BMI270_CS_PIN`, `IMU_BMI270_ALIGN` |
| `USE_ACCGYRO_LSM6DSK320X` / `LSM6DSV16X` | `USE_IMU_LSM6DXX` + `LSM6DXX_SPI_BUS`, `LSM6DXX_CS_PIN`, `IMU_LSM6DXX_ALIGN` (these two chips need INAV 9.1 or newer) |
| `GYRO_1_ALIGN CW180_DEG` | `IMU_xxx_ALIGN CW180_DEG` (same names) |
| `GYRO_1_SPI_INSTANCE SPI1` | `xxx_SPI_BUS BUS_SPI1` and `USE_SPI_DEVICE_1` |
| `SPIn_SCK/SDI/SDO_PIN` | `SPIn_SCK_PIN`, `SPIn_MISO_PIN` (= SDI), `SPIn_MOSI_PIN` (= SDO) |
| `UARTn_TX/RX_PIN` | `USE_UARTn` + `UARTn_TX_PIN`, `UARTn_RX_PIN`; set `SERIAL_PORT_COUNT` = UARTs + 1 (VCP) |
| `USE_FLASH_M25P16`, `FLASH_CS_PIN` | `USE_FLASHFS`, `USE_FLASH_M25P16`, `M25P16_SPI_BUS`, `M25P16_CS_PIN` |
| `USE_MAX7456`, `MAX7456_SPI_CS_PIN` | `USE_MAX7456`, `MAX7456_SPI_BUS`, `MAX7456_CS_PIN` |
| `I2Cn_SCL/SDA_PIN` | `USE_I2C`, `USE_I2C_DEVICE_n`, `I2Cn_SCL`, `I2Cn_SDA` |
| `BARO_I2C_INSTANCE` | `USE_BARO`, `BARO_I2C_BUS BUS_I2Cn`, `USE_BARO_BMP280` / `USE_BARO_DPS310` ... |
| `ADC_VBAT_PIN`, `ADC_CURR_PIN` | `USE_ADC`, `ADC_CHANNEL_1_PIN`, `ADC_CHANNEL_2_PIN`, `VBAT_ADC_CHANNEL ADC_CHN_1`, `CURRENT_METER_ADC_CHANNEL ADC_CHN_2` |
| `DEFAULT_CURRENT_METER_SCALE` | `CURRENT_METER_SCALE` |
| `LED0_PIN`, `LED1_PIN` | `LED0`, `LED1` |
| `BEEPER_PIN`, `BEEPER_INVERTED` | `BEEPER`, `BEEPER_INVERTED` |
| `LED_STRIP_PIN` | `USE_LED_STRIP`, `WS2811_PIN` (and a `TIM_USE_LED` entry in `target.c`) |
| `MOTORn_PIN`, `TIMER_PIN_MAPPING` | entries in `target.c` (below) |

Also set `MAX_PWM_OUTPUT_PORTS`, `TARGET_IO_PORTA/B/C` (`0xffff`) and, for 48-pin parts,
`TARGET_IO_PORTF (BIT(0) | BIT(1))`.

**Pin alternate functions are looked up automatically** for SPI1-4, USART1-3, UART4/5 and all
timer channels. I2C uses AF4 (I2C1/2), AF8 (I2C3), AF3 (I2C4); if a board uses an I2C pin with a
different AF, set `I2Cn_AF` in `target.h`.

**ADC:** pins on ADC1 (PA0-PA3, PB0, PB1, PB11, PB12, PB14, PC0-PC3, PF0) work as is. Pins only
on ADC2 (PA4-PA7, PB2, PB15, PC4, PC5, PF1) also need `#define ADC_CHANNEL_n_INSTANCE ADC2`.

### `target.c` — motors and LED strip

```c
timerHardware_t timerHardware[] = {
    DEF_TIM(TIM3, CH3, PB0, TIM_USE_MOTOR, 0, 0),   // M1, DMA1 channel 1
    DEF_TIM(TIM3, CH4, PB1, TIM_USE_MOTOR, 0, 1),   // M2, DMA1 channel 2
    ...
    DEF_TIM(TIM5, CH1, PB2, TIM_USE_LED,   0, 4),   // LED strip, DMA1 channel 5
};
```

* **Which timer:** Betaflight's `TIMER_PIN_MAP(i, pin, option, dma)` picks the *option*-th entry
  (1-based) for that pin in Betaflight's G4 timer table, `src/platform/STM32/timer_stm32g4xx.c`
  (`fullTimerHardware[]`). Example: `TIMER_PIN_MAP(2, PB6, 3, 4)` → third PB6 entry → `TIM8, CH1`.
  Complementary outputs are written `CH1N`...`CH4N`.
* **DMA (last argument):** 0-7 = DMA1 channel 1-8, 8-15 = DMA2 channel 1-8. The G4 DMAMUX routes
  any timer to any channel, so just give every output its own number. **Do not use 8 or 9**:
  DMA2 channels 1 and 2 belong to the ADC.
* Motors that share a timer must run the same protocol/rate (normal for DShot).

### `config.c` (optional)

Board defaults applied on first boot; needs `#define USE_TARGET_CONFIG` in `target.h`. The Matrix
targets use it to default to DShot300.

## Build

Needs the Arm GNU toolchain **13.2** (`arm-none-eabi-gcc`), CMake and Ruby:

```sh
mkdir -p build && cd build
cmake ..
make YOURTARGET            # -> inav_<version>_YOURTARGET.hex
```

Check the memory summary at the end of the link: `RAM` is the 96 KB main SRAM, `CCM` the 32 KB
core-coupled RAM (stack and fast variables).

## Flash

DFU mode (BOOT button while plugging in USB, or `bl` in the Betaflight CLI / `dfu` in INAV),
then INAV Configurator → Firmware Flasher → *Load Firmware [Local]* with *Full chip erase*, or
STM32CubeProgrammer. Connect afterwards with the Configurator version matching this tree's INAV
version.
