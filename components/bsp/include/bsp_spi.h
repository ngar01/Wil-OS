/**
 * @file bsp_spi.h
 * @brief Shared SPI Bus Manager with Mutex Protection
 * 
 * Manages access to the shared SPI bus between Display and SD Card.
 * Implements mutex-based arbitration to prevent data corruption.
 * 
 * @author Wil-OS Team
 * @version 1.0.0
 */

#ifndef BSP_SPI_H
#define BSP_SPI_H

#include "bsp_common.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief SPI device handle (opaque type)
 */
typedef spi_device_handle_t bsp_spi_device_t;

/**
 * @brief SPI transaction statistics
 */
typedef struct {
    uint32_t total_transactions;    ///< Total number of transactions
    uint32_t failed_transactions;   ///< Failed transaction count
    uint32_t mutex_timeouts;        ///< Mutex acquisition timeouts
    uint32_t max_wait_time_ms;      ///< Maximum mutex wait time
    uint64_t total_bytes_tx;        ///< Total bytes transmitted
    uint64_t total_bytes_rx;        ///< Total bytes received
} bsp_spi_stats_t;

/**
 * @brief Initialize SPI bus and create mutex
 * 
 * Configures:
 *   - SPI2 host with DMA
 *   - MOSI, MISO, SCLK pins
 *   - Priority inheritance mutex
 * 
 * @return 
 *   - ESP_OK: Success
 *   - ESP_ERR_INVALID_STATE: Already initialized
 *   - ESP_FAIL: SPI initialization failed
 */
esp_err_t bsp_spi_init(void);

/**
 * @brief Deinitialize SPI bus
 * 
 * @warning All devices must be removed before calling this
 * 
 * @return 
 *   - ESP_OK: Success
 *   - ESP_ERR_INVALID_STATE: Not initialized or devices still attached
 */
esp_err_t bsp_spi_deinit(void);

/**
 * @brief Add a device to the SPI bus
 * 
 * @param[in] cs_pin Chip select GPIO pin
 * @param[in] clock_speed_hz SPI clock frequency
 * @param[in] mode SPI mode (0-3)
 * @param[out] out_handle Device handle (for transactions)
 * 
 * @return 
 *   - ESP_OK: Success
 *   - ESP_ERR_INVALID_ARG: Invalid parameters
 *   - ESP_ERR_NO_MEM: Cannot allocate device
 */
esp_err_t bsp_spi_add_device(gpio_num_t cs_pin, 
                              uint32_t clock_speed_hz,
                              uint8_t mode,
                              bsp_spi_device_t *out_handle);

/**
 * @brief Remove a device from the SPI bus
 * 
 * @param[in] device Device handle
 * @return 
 *   - ESP_OK: Success
 *   - ESP_ERR_INVALID_ARG: Invalid handle
 */
esp_err_t bsp_spi_remove_device(bsp_spi_device_t device);

/**
 * @brief Acquire SPI bus mutex
 * 
 * Must be called before any SPI transaction.
 * Prevents concurrent access from other tasks.
 * 
 * @param[in] timeout_ms Timeout in milliseconds (use portMAX_DELAY for infinite)
 * @return 
 *   - ESP_OK: Mutex acquired
 *   - ESP_ERR_TIMEOUT: Timeout expired
 * 
 * @note Always pair with bsp_spi_release()
 */
esp_err_t bsp_spi_acquire(uint32_t timeout_ms);

/**
 * @brief Release SPI bus mutex
 * 
 * Must be called after completing SPI transactions.
 * 
 * @return 
 *   - ESP_OK: Mutex released
 *   - ESP_FAIL: Mutex not held by current task
 */
esp_err_t bsp_spi_release(void);

/**
 * @brief Perform a blocking SPI transaction
 * 
 * @warning Must hold SPI mutex (call bsp_spi_acquire first)
 * 
 * @param[in] device Device handle
 * @param[in,out] trans Transaction descriptor
 * @return 
 *   - ESP_OK: Success
 *   - ESP_ERR_INVALID_STATE: Mutex not held
 *   - ESP_FAIL: Transaction failed
 */
esp_err_t bsp_spi_transmit(bsp_spi_device_t device, spi_transaction_t *trans);

/**
 * @brief Queue an asynchronous SPI transaction
 * 
 * @warning Must hold SPI mutex
 * 
 * @param[in] device Device handle
 * @param[in] trans Transaction descriptor
 * @param[in] timeout_ms Queue timeout
 * @return 
 *   - ESP_OK: Transaction queued
 *   - ESP_ERR_TIMEOUT: Queue full
 */
esp_err_t bsp_spi_queue_trans(bsp_spi_device_t device, 
                               spi_transaction_t *trans,
                               uint32_t timeout_ms);

/**
 * @brief Wait for a queued transaction to complete
 * 
 * @param[in] device Device handle
 * @param[out] out_trans Completed transaction (can be NULL)
 * @param[in] timeout_ms Wait timeout
 * @return 
 *   - ESP_OK: Transaction completed
 *   - ESP_ERR_TIMEOUT: Timeout expired
 */
esp_err_t bsp_spi_get_trans_result(bsp_spi_device_t device,
                                    spi_transaction_t **out_trans,
                                    uint32_t timeout_ms);

/**
 * @brief Get SPI bus statistics
 * 
 * @param[out] stats Statistics structure to fill
 * @return ESP_OK on success
 */
esp_err_t bsp_spi_get_stats(bsp_spi_stats_t *stats);

/**
 * @brief Reset SPI bus statistics
 * 
 * @return ESP_OK on success
 */
esp_err_t bsp_spi_reset_stats(void);

/**
 * @brief Check if SPI mutex is currently held
 * 
 * @return true if held, false otherwise
 */
bool bsp_spi_is_acquired(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_SPI_H */
