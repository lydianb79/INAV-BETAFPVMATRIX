# STM32G4 (Cortex-M4F) support.
# HAL/LL/CMSIS sources come from lib/main/STM32G4 (STM32CubeG4, as used by Betaflight).

include(cortex-m4f)
include(stm32g4-usb)

set(STM32G4_CMSIS_DEVICE_DIR "${MAIN_LIB_DIR}/main/STM32G4/Drivers/CMSIS/Device/ST/STM32G4xx")
set(STM32G4_HAL_DIR "${MAIN_LIB_DIR}/main/STM32G4/Drivers/STM32G4xx_HAL_Driver")

set(STM32G4_HAL_SRC
    stm32g4xx_hal.c
    stm32g4xx_hal_adc.c
    stm32g4xx_hal_adc_ex.c
    stm32g4xx_hal_cortex.c
    stm32g4xx_hal_dma.c
    stm32g4xx_hal_dma_ex.c
    stm32g4xx_hal_exti.c
    stm32g4xx_hal_flash.c
    stm32g4xx_hal_flash_ex.c
    stm32g4xx_hal_flash_ramfunc.c
    stm32g4xx_hal_gpio.c
    stm32g4xx_hal_i2c.c
    stm32g4xx_hal_i2c_ex.c
    stm32g4xx_hal_pcd.c
    stm32g4xx_hal_pcd_ex.c
    stm32g4xx_hal_pwr.c
    stm32g4xx_hal_pwr_ex.c
    stm32g4xx_hal_rcc.c
    stm32g4xx_hal_rcc_ex.c
    stm32g4xx_hal_rtc.c
    stm32g4xx_hal_rtc_ex.c
    stm32g4xx_hal_spi.c
    stm32g4xx_hal_spi_ex.c
    stm32g4xx_hal_tim.c
    stm32g4xx_hal_tim_ex.c
    stm32g4xx_hal_uart.c
    stm32g4xx_hal_uart_ex.c
    stm32g4xx_ll_dma.c
    stm32g4xx_ll_exti.c
    stm32g4xx_ll_gpio.c
    stm32g4xx_ll_i2c.c
    stm32g4xx_ll_rcc.c
    stm32g4xx_ll_spi.c
    stm32g4xx_ll_tim.c
    stm32g4xx_ll_usart.c
    stm32g4xx_ll_usb.c
)

list(TRANSFORM STM32G4_HAL_SRC PREPEND "${STM32G4_HAL_DIR}/Src/")

set(STM32G4_VCP_DIR "${MAIN_SRC_DIR}/vcp_hal")

set(STM32G4_VCP_SRC
    usbd_desc.c
    usbd_conf_stm32g4xx.c
    usbd_cdc_interface.c
)
list(TRANSFORM STM32G4_VCP_SRC PREPEND "${STM32G4_VCP_DIR}/")

set(STM32G4_INCLUDE_DIRS
    ${STM32G4_HAL_DIR}/Inc
    ${STM32G4_CMSIS_DEVICE_DIR}/Include
)

main_sources(STM32G4_SRC
    target/system_stm32g4xx.c

    config/config_streamer_stm32g4.c
    config/config_streamer_ram.c
    config/config_streamer_extflash.c

    drivers/adc_stm32g4xx.c
    drivers/bus_i2c_hal.c
    drivers/dma_stm32g4xx.c
    drivers/bus_spi_hal_ll.c
    drivers/timer.c
    drivers/timer_impl_hal.c
    drivers/timer_stm32g4xx.c
    drivers/system_stm32g4xx.c
    drivers/serial_uart_stm32g4xx.c
    drivers/serial_uart_hal.c
)

set(STM32G4_DEFINITIONS
    ${CORTEX_M4F_DEFINITIONS}
    USE_HAL_DRIVER
    USE_FULL_LL_DRIVER
)

function(target_stm32g4xx)
    # Vendor HAL sources we don't control
    set_source_files_properties(
        ${STM32G4_HAL_SRC}
        DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        PROPERTIES COMPILE_OPTIONS "-Wno-unused-parameter"
    )
    target_stm32(
        SOURCES ${STM32G4_HAL_SRC} ${STM32G4_SRC}
        COMPILE_DEFINITIONS ${STM32G4_DEFINITIONS}
        COMPILE_OPTIONS ${CORTEX_M4F_COMMON_OPTIONS} ${CORTEX_M4F_COMPILE_OPTIONS}
        SYSTEM_INCLUDE_DIRECTORIES ${STM32G4_INCLUDE_DIRS}
        LINK_OPTIONS ${CORTEX_M4F_COMMON_OPTIONS} ${CORTEX_M4F_LINK_OPTIONS}

        # USB MSC (mass storage) is not ported to the G4 USB FS device peripheral yet
        DISABLE_MSC
        VCP_SOURCES ${STM32G4_USB_SRC} ${STM32G4_VCP_SRC}
        VCP_INCLUDE_DIRECTORIES ${STM32G4_USB_INCLUDE_DIRS} ${STM32G4_VCP_DIR}

        OPENOCD_TARGET stm32g4x

        ${ARGN}
    )
endfunction()

# target_stm32g474xe / target_stm32g473xe: 512 KB flash, 128 KB RAM (96 KB SRAM + 32 KB CCM)
macro(define_target_stm32g4 subfamily size ram)
    function(target_stm32g4${subfamily}x${size} name)
        set(func_ARGV ARGV)
        string(TOUPPER ${size} upper_size)
        get_stm32_flash_size(flash_size ${size})
        set(definitions
            STM32G4
            STM32G474xx     # G473 is the G474 without the HRTIM; they share the CMSIS header
            STM32G4${subfamily}x${upper_size}
            MCU_FLASH_SIZE=${flash_size}
            MCU_RAM_SIZE=${ram}
        )
        target_stm32g4xx(
            NAME ${name}
            STARTUP startup_stm32g474xx.s
            COMPILE_DEFINITIONS ${definitions}
            LINKER_SCRIPT stm32_flash_g474x${size}
            OPTIMIZATION -Os

            ${${func_ARGV}}
        )
    endfunction()
endmacro()

define_target_stm32g4(74 e 128)
define_target_stm32g4(73 e 128)
