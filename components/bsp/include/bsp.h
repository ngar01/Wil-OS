/**
 * @file bsp.h
 * @brief Main BSP Header - Public API
 * 
 * Include this file in main.c to use BSP functionality
 * 
 * @author Wil-OS Team
 * @version 1.0.0
 */

#ifndef BSP_H
#define BSP_H

#include "bsp_common.h"
#include "bsp_gpio.h"
#include "bsp_spi.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the Board Support Package
 * 
 * This is the main entry point for BSP initialization.
 * Call this from main.c before initializing HAL drivers.
 * 
 * @param[in] config Configuration structure (NULL for defaults)
 * @return 
 *   - ESP_OK: Success
 *   - ESP_ERR_INVALID_STATE: Already initialized
 *   - ESP_FAIL: Initialization failed
 * 
 * @note This function must be called before any HAL operations
 */
esp_err_t bsp_init(const bsp_config_t *config);

/**
 * @brief Deinitialize the Board Support Package
 * 
 * @return 
 *   - ESP_OK: Success
 *   - ESP_ERR_INVALID_STATE: Not initialized
 */
esp_err_t bsp_deinit(void);

/**
 * @brief Check if BSP is initialized
 * 
 * @return true if initialized, false otherwise
 */
bool bsp_is_initialized(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_H */
