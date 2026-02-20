/**
 * @file bsp_gpio.h
 * @brief GPIO Initialization and Management
 * 
 * Configures all GPIO pins used by Wil-OS peripherals.
 * 
 * @author Wil-OS Team
 * @version 1.0.0
 */

#ifndef BSP_GPIO_H
#define BSP_GPIO_H

#include "bsp_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize all GPIO pins
 * 
 * Configures:
 *   - Display control pins (DC, RST, BL)
 *   - SD card control pins (CS)
 *   - Keypad matrix pins (rows as outputs, columns as inputs)
 * 
 * @note SPI data pins (MOSI, MISO, SCLK) are configured by SPI driver
 * 
 * @return 
 *   - ESP_OK: Success
 *   - ESP_FAIL: GPIO configuration failed
 */
esp_err_t bsp_gpio_init(void);

/**
 * @brief Set GPIO pin high
 * 
 * @param[in] pin GPIO pin number
 * @return 
 *   - ESP_OK: Success
 *   - ESP_ERR_INVALID_ARG: Invalid pin
 */
esp_err_t bsp_gpio_set_high(gpio_num_t pin);

/**
 * @brief Set GPIO pin low
 * 
 * @param[in] pin GPIO pin number
 * @return 
 *   - ESP_OK: Success
 *   - ESP_ERR_INVALID_ARG: Invalid pin
 */
esp_err_t bsp_gpio_set_low(gpio_num_t pin);

/**
 * @brief Read GPIO pin level
 * 
 * @param[in] pin GPIO pin number
 * @return 1 if high, 0 if low, -1 on error
 */
int bsp_gpio_get_level(gpio_num_t pin);

/**
 * @brief Toggle GPIO pin
 * 
 * @param[in] pin GPIO pin number
 * @return 
 *   - ESP_OK: Success
 *   - ESP_ERR_INVALID_ARG: Invalid pin
 */
esp_err_t bsp_gpio_toggle(gpio_num_t pin);

/**
 * @brief Configure display backlight PWM
 * 
 * @param[in] brightness Brightness level (0-100%)
 * @return 
 *   - ESP_OK: Success
 *   - ESP_ERR_INVALID_ARG: Invalid brightness
 */
esp_err_t bsp_gpio_set_backlight(uint8_t brightness);

#ifdef __cplusplus
}
#endif

#endif /* BSP_GPIO_H */
