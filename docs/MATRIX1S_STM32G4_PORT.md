# BETAFPV Matrix 1S (3IN1 / 4IN1) for INAV — STM32G4 port

> **Experimental, untested on hardware.** These targets build, but nobody has flown them yet.
> Bench-test with **propellers removed** and check every item in the checklist below
> before flying.

## What is in this tree

Upstream INAV (`iNavFlight/inav`, master at `b006087`, 1 Nov 2026) has no STM32G4 support.
The Matrix 1S boards use an **STM32G473CEU6**, so this tree adds:

1. **STM32G4 platform support** (new MCU family), modelled on INAV's H7 port and
   Betaflight's G4 code:
   - `lib/main/STM32G4` — STM32CubeG4 HAL/LL/CMSIS and the USB device library (taken from Betaflight)
   - `cmake/stm32g4.cmake`, `cmake/stm32g4-usb.cmake` — `target_stm32g473xe()` / `target_stm32g474xe()`
   - `src/main/startup/startup_stm32g474xx.s`, `src/main/target/link/stm32_flash_g474xe.ld`
   - `src/main/target/system_stm32g4xx.c/.h`, `stm32g4xx_hal_conf.h` — 168 MHz clock from HSE or HSI16
   - Drivers: `adc_stm32g4xx.c`, `dma_stm32g4xx.c`, `serial_uart_stm32g4xx.c`,
     `system_stm32g4xx.c`, `timer_stm32g4xx.c`, `timer_def_stm32g4xx.h`, `bus_spi_stm32g4xx.h`,
     `config/config_streamer_stm32g4.c`, `vcp_hal/usbd_conf_stm32g4xx.c`
   - `STM32G4` branches in the shared drivers (RCC, IO, EXTI, SPI, I2C, DMA, timer DMA, USB VCP,
     persistent storage, bootloader jump). Every change to a shared file is guarded by
     `defined(STM32G4)`; F4/F7/H7/AT32 builds are unchanged.
2. **ICM42622P** gyro detection (WHO_AM_I 0x46, handled like the ICM42688P as in Betaflight),
   because BETAFPV ships some 4IN1 boards with it.
3. Two targets:

| INAV target     | Board                                   | Betaflight source target | Clock        |
|-----------------|-----------------------------------------|--------------------------|--------------|
| `BETAFPVMATRIX_3IN1` | Matrix 1S Brushless FC 3IN1 (HD)        | `BETAFPVG473`            | HSI16 (no crystal) |
| `BETAFPVMATRIX_4IN1` | Matrix 1S Brushless FC 4IN1 (analog)    | `BETAFPVG473_V2`         | 8 MHz HSE    |

BETAFPV specifies `BETAFPVG473` for the 3IN1 and `BETAFPVG473_V2` for the 4IN1; the two
pinouts differ, so **flash the matching file**.

## Pin map

|                 | 3IN1 (`BETAFPVMATRIX_3IN1`)          | 4IN1 (`BETAFPVMATRIX_4IN1`)                 |
|-----------------|---------------------------------|----------------------------------------|
| Gyro (SPI1)     | CS PA4 — ICM42688P / BMI270      | CS PC14 — BMI270 / ICM42688P / ICM42622P / LSM6DSK320X |
| Flash (SPI3)    | CS PB9                          | CS PC13                                |
| OSD (SPI2)      | — (no analog OSD)               | AT7456E, CS PB12                       |
| Motor 1..4      | PB0 TIM3_CH3, PB1 TIM3_CH4, PB6 TIM8_CH1, PC13 TIM8_CH4N | PB0 TIM3_CH3, PB1 TIM3_CH4, PC6 TIM3_CH1, PA4 TIM3_CH2 |
| LED strip       | PB2 TIM5_CH1                    | PB2 TIM5_CH1                           |
| UART1 / 2 / 4   | PA9/PA10, PA2/PA3, PC10/PC11    | same                                   |
| UART3           | PB10/PB11 — onboard ELRS (CRSF) | same                                   |
| I2C1            | SCL PA15 / SDA PB7 (baro/mag)   | same                                   |
| VBAT / Current  | PA0 / PA1 (scale 750)           | same                                   |
| Beeper          | PA8 (inverted)                  | same                                   |
| LEDs            | PC15, PC14                      | PB6, PC4                               |

Gyro alignment is `CW180_DEG`, as in Betaflight. Motor protocol defaults to DShot300
(BB51 Bluejay ESCs). Motor DMA uses DMA1 channels 1–4, the LED strip DMA1 channel 5,
the ADC DMA2 channels 1–2.

## Building

Needs the Arm GNU toolchain 13.2 (`arm-none-eabi-gcc`), CMake and Ruby, like any INAV build:

```sh
mkdir build && cd build
cmake ..
make BETAFPVMATRIX_3IN1      # -> inav_10.0.0_BETAFPVMATRIX_3IN1.hex
make BETAFPVMATRIX_4IN1      # -> inav_10.0.0_BETAFPVMATRIX_4IN1.hex
```

## Flashing

Put the board in DFU mode (hold BOOT while plugging in USB, or `dfu` in the CLI of the
current firmware), then flash the `.hex` with INAV Configurator → Firmware Flasher →
**Load Firmware [Local]**, with **Full chip erase** enabled. If the Configurator does not
handle the G4 DFU device, use STM32CubeProgrammer (USB/DFU) instead. Make a Betaflight
`diff all` backup first so you can go back.

## Bench checklist (props off!)

1. Board enumerates as a USB serial port and INAV Configurator connects.
2. `status` in the CLI shows ~168 MHz and no hardware errors; gyro/acc detected and the
   3D model moves the right way (alignment).
3. `save` + reboot: settings persist (internal-flash config storage).
4. Receiver tab: ELRS channels move (UART3, CRSF). If not, the onboard receiver may need
   binding / a matching ELRS version.
5. Battery voltage and current read plausibly (adjust `vbat_scale` / `current_meter_scale`).
6. Motors tab, **props off**: each motor spins individually, in the right position and
   direction. Check DShot works (especially motor 4 on the 3IN1, which uses the
   G4-only complementary output TIM8_CH4N on PC13).
7. Blackbox flash is detected (Blackbox tab), OSD shows on the 4IN1.
8. `dfu` CLI command reboots into the ROM bootloader.

## Known gaps

- USB mass storage (MSC) is disabled on G4; blackbox is downloaded over the Configurator.
- No INAV bootloader (`_bl`/`_for_bl`) variants, no SD card, no DroneCAN, no softserial on G4.
- On the 3IN1, connect your HD VTX's UART and set it to *MSP DisplayPort* in the Ports tab.
