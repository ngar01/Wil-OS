/**
 * @file bsp_common.h
 * @brief Board Support Package - Common Definitions and Hardware Mapping
 * * This file centralizes all hardware-specific definitions for Wil-OS.
 * Changing target hardware only requires updating this file.
 * * @author Wil-OS Team
 * @version 1.0.0
 */

#ifndef BSP_COMMON_H
#define BSP_COMMON_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "esp_timer.h"    // Added to fix esp_timer_get_time error
#include "driver/gpio.h"
#include "driver/spi_master.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * HARDWARE VERSION
 * ======================================================================== */

#define BSP_HARDWARE_VERSION_MAJOR  1
#define BSP_HARDWARE_VERSION_MINOR  0
#define BSP_HARDWARE_VERSION_PATCH  0

/* ========================================================================
 * GPIO PIN MAPPING - ST7735 DISPLAY (SPI)
 * ======================================================================== */

#define BSP_DISPLAY_SPI_HOST        SPI2_HOST       ///< SPI2 (HSPI on ESP32-S3)
#define BSP_DISPLAY_MOSI_PIN        GPIO_NUM_11     ///< SPI MOSI (Master Out Slave In)
#define BSP_DISPLAY_SCLK_PIN        GPIO_NUM_12     ///< SPI Clock
#define BSP_DISPLAY_CS_PIN          GPIO_NUM_10     ///< Chip Select (active low)
#define BSP_DISPLAY_DC_PIN          GPIO_NUM_14     ///< Data/Command select
#define BSP_DISPLAY_RST_PIN         GPIO_NUM_17     ///< Reset (active low)
#define BSP_DISPLAY_BL_PIN          GPIO_NUM_21     ///< Backlight (PWM capable)

// Display SPI configuration
#define BSP_DISPLAY_SPI_FREQ_HZ     40000000        ///< 40 MHz (ST7735 max: 15MHz write, 6.6MHz read)
#define BSP_DISPLAY_WIDTH           128             ///< Display width in pixels
#define BSP_DISPLAY_HEIGHT          160             ///< Display height in pixels

/* ========================================================================
 * GPIO PIN MAPPING - SD CARD (SPI, SHARED BUS)
 * ======================================================================== */

#define BSP_SDCARD_SPI_HOST         SPI2_HOST       ///< Same host as display (shared bus)
#define BSP_SDCARD_MOSI_PIN         GPIO_NUM_11     ///< Shared with display
#define BSP_SDCARD_MISO_PIN         GPIO_NUM_13     ///< MISO (Master In Slave Out)
#define BSP_SDCARD_SCLK_PIN         GPIO_NUM_12     ///< Shared with display
#define BSP_SDCARD_CS_PIN           GPIO_NUM_9      ///< Chip Select (different from display!)

// SD Card SPI configuration
#define BSP_SDCARD_SPI_FREQ_HZ      20000000        ///< 20 MHz (SD card typically 20-25 MHz)

/* ========================================================================
 * GPIO PIN MAPPING - 4x4 KEYPAD MATRIX
 * ======================================================================== */

// Row pins (outputs, driven by ESP32)
#define BSP_KEYPAD_ROW1_PIN         GPIO_NUM_1
#define BSP_KEYPAD_ROW2_PIN         GPIO_NUM_2
#define BSP_KEYPAD_ROW3_PIN         GPIO_NUM_3
#define BSP_KEYPAD_ROW4_PIN         GPIO_NUM_4

// Column pins (inputs with pull-ups)
#define BSP_KEYPAD_COL1_PIN         GPIO_NUM_5
#define BSP_KEYPAD_COL2_PIN         GPIO_NUM_6
#define BSP_KEYPAD_COL3_PIN         GPIO_NUM_7
#define BSP_KEYPAD_COL4_PIN         GPIO_NUM_8

// Keypad scanning parameters
#define BSP_KEYPAD_SCAN_PERIOD_MS   20              ///< Scan every 20ms
#define BSP_KEYPAD_DEBOUNCE_MS      50              ///< 50ms debounce time

/* ========================================================================
 * DMA CONFIGURATION
 * ======================================================================== */

#define BSP_SPI_DMA_CHAN            SPI_DMA_CH_AUTO ///< Auto-allocate DMA channel
#define BSP_SPI_MAX_TRANSFER_SIZE   (128 * 160 * 2) ///< Max DMA transfer (framebuffer)

/* ========================================================================
 * POWER MANAGEMENT
 * ======================================================================== */

#define BSP_POWER_ENABLE_PIN        GPIO_NUM_NC     ///< Not connected (no power switch)
#define BSP_BATTERY_ADC_CHANNEL     ADC1_CHANNEL_0  ///< For battery monitoring (future)

/* ========================================================================
 * LED INDICATORS (Optional, for debugging)
 * ======================================================================== */

#define BSP_LED_STATUS_PIN          GPIO_NUM_NC     ///< Status LED (not used)
#define BSP_LED_ERROR_PIN           GPIO_NUM_NC     ///< Error LED (not used)

/* ========================================================================
 * TIMING CONSTRAINTS
 * ======================================================================== */

#define BSP_SPI_MUTEX_TIMEOUT_MS    100             ///< Max wait for SPI bus
#define BSP_DISPLAY_RESET_PULSE_MS  10              ///< Reset pulse duration
#define BSP_DISPLAY_INIT_DELAY_MS   120             ///< Post-reset init delay

/* ========================================================================
 * MEMORY ALLOCATION FLAGS
 * ======================================================================== */

#define BSP_MALLOC_CAP_DEFAULT      (MALLOC_CAP_DEFAULT)
#define BSP_MALLOC_CAP_DMA          (MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL)
#define BSP_MALLOC_CAP_SPIRAM       (MALLOC_CAP_SPIRAM)

/* ========================================================================
 * TYPE DEFINITIONS
 * ======================================================================== */

/**
 * @brief BSP initialization configuration
 */
typedef struct {
    bool init_gpio;         ///< Initialize GPIO subsystem
    bool init_spi;          ///< Initialize SPI bus
    bool init_power;        ///< Initialize power management
    uint32_t cpu_freq_mhz;  ///< CPU frequency (0 = use default)
} bsp_config_t;

/**
 * @brief Default BSP configuration
 */
#define BSP_CONFIG_DEFAULT() { \
    .init_gpio = true,         \
    .init_spi = true,          \
    .init_power = false,       \
    .cpu_freq_mhz = 0          \
}

/* ========================================================================
 * UTILITY MACROS
 * ======================================================================== */

/**
 * @brief Check if a GPIO pin is valid
 */
#define BSP_GPIO_IS_VALID(pin)  ((pin) >= 0 && (pin) < GPIO_NUM_MAX)

/**
 * @brief Convert milliseconds to FreeRTOS ticks
 */
#define BSP_MS_TO_TICKS(ms)     (pdMS_TO_TICKS(ms))

/**
 * @brief Get array size
 */
#define BSP_ARRAY_SIZE(arr)     (sizeof(arr) / sizeof((arr)[0]))

/**
 * @brief Minimum of two values
 */
#define BSP_MIN(a, b)           ((a) < (b) ? (a) : (b))

/**
 * @brief Maximum of two values
 */
#define BSP_MAX(a, b)           ((a) > (b) ? (a) : (b))

/**
 * @brief Clamp value between min and max
 */
#define BSP_CLAMP(val, min, max) (BSP_MAX((min), BSP_MIN((val), (max))))

/* ========================================================================
 * LOGGING TAGS
 * ======================================================================== */

#define BSP_TAG         "BSP"
#define BSP_GPIO_TAG    "BSP_GPIO"
#define BSP_SPI_TAG     "BSP_SPI"
#define BSP_POWER_TAG   "BSP_POWER"

/* ========================================================================
 * ERROR CODES (Custom)
 * ======================================================================== */

#define BSP_ERR_BASE                0x50000     ///< BSP error base
#define BSP_ERR_NOT_INITIALIZED     (BSP_ERR_BASE + 1)
#define BSP_ERR_ALREADY_INITIALIZED (BSP_ERR_BASE + 2)
#define BSP_ERR_INVALID_ARG         (BSP_ERR_BASE + 3)
#define BSP_ERR_TIMEOUT             (BSP_ERR_BASE + 4)
#define BSP_ERR_NO_MEM              (BSP_ERR_BASE + 5)

/* ========================================================================
 * COMPILE-TIME ASSERTIONS
 * ======================================================================== */

// Ensure display and SD card use different CS pins
_Static_assert(BSP_DISPLAY_CS_PIN != BSP_SDCARD_CS_PIN, 
               "Display and SD card must have different CS pins!");

// Ensure SPI frequency is reasonable
_Static_assert(BSP_DISPLAY_SPI_FREQ_HZ <= 80000000, 
               "SPI frequency too high for ESP32-S3!");

// Ensure keypad has valid pin count
_Static_assert(BSP_GPIO_IS_VALID(BSP_KEYPAD_ROW1_PIN), 
               "Invalid keypad row pin!");

#ifdef __cplusplus
}
#endif

#endif /* BSP_COMMON_H */