/**
 * @file bsp_spi.c
 * @brief Shared SPI Bus Manager Implementation
 * 
 * Critical component: Manages contention between Display and SD Card
 * 
 * @author Wil-OS Team
 * @version 1.0.0
 */

#include "bsp_spi.h"
#include "esp_log.h"
#include "string.h"

static const char *TAG = BSP_SPI_TAG;

// ========================================================================
// Module State
// ========================================================================

static bool spi_initialized = false;
static spi_bus_config_t bus_config;
static SemaphoreHandle_t spi_mutex = NULL;
static TaskHandle_t mutex_holder = NULL;
static bsp_spi_stats_t stats = {0};

esp_err_t bsp_spi_init(void)
{
    if (spi_initialized) {
        ESP_LOGW(TAG, "SPI bus already initialized");
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "Initializing SPI bus (host=%d)", BSP_DISPLAY_SPI_HOST);

    // ========================================================================
    // Step 1: Create mutex with priority inheritance
    // ========================================================================

    spi_mutex = xSemaphoreCreateMutex();
    if (spi_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create SPI mutex");
        return ESP_ERR_NO_MEM;
    }
    
    ESP_LOGI(TAG, "✓ SPI mutex created (priority inheritance enabled)");

    // ========================================================================
    // Step 2: Configure SPI bus
    // ========================================================================

    memset(&bus_config, 0, sizeof(bus_config));
    
    bus_config.mosi_io_num = BSP_DISPLAY_MOSI_PIN;
    bus_config.miso_io_num = BSP_SDCARD_MISO_PIN;  // Display has no MISO
    bus_config.sclk_io_num = BSP_DISPLAY_SCLK_PIN;
    bus_config.quadwp_io_num = -1;                 // Not used
    bus_config.quadhd_io_num = -1;                 // Not used
    bus_config.max_transfer_sz = BSP_SPI_MAX_TRANSFER_SIZE;
    bus_config.flags = SPICOMMON_BUSFLAG_MASTER;
    
    ESP_LOGI(TAG, "SPI bus config:");
    ESP_LOGI(TAG, "  MOSI: GPIO%d", bus_config.mosi_io_num);
    ESP_LOGI(TAG, "  MISO: GPIO%d", bus_config.miso_io_num);
    ESP_LOGI(TAG, "  SCLK: GPIO%d", bus_config.sclk_io_num);
    ESP_LOGI(TAG, "  Max transfer: %d bytes", bus_config.max_transfer_sz);

    // ========================================================================
    // Step 3: Initialize SPI bus with DMA
    // ========================================================================

    esp_err_t ret = spi_bus_initialize(BSP_DISPLAY_SPI_HOST, 
                                       &bus_config, 
                                       BSP_SPI_DMA_CHAN);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI bus init failed: %s", esp_err_to_name(ret));
        vSemaphoreDelete(spi_mutex);
        spi_mutex = NULL;
        return ret;
    }

    spi_initialized = true;
    memset(&stats, 0, sizeof(stats)); // Reset stats
    
    ESP_LOGI(TAG, "✓ SPI bus initialized with DMA (channel=AUTO)");
    return ESP_OK;
}

esp_err_t bsp_spi_deinit(void)
{
    if (!spi_initialized) {
        ESP_LOGW(TAG, "SPI bus not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "De-initializing SPI bus...");

    esp_err_t ret = spi_bus_free(BSP_DISPLAY_SPI_HOST);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "SPI bus free failed: %s", esp_err_to_name(ret));
        // Continue anyway
    }

    if (spi_mutex != NULL) {
        vSemaphoreDelete(spi_mutex);
        spi_mutex = NULL;
    }

    spi_initialized = false;
    mutex_holder = NULL;
    
    ESP_LOGI(TAG, "✓ SPI bus deinitialized");
    return ESP_OK;
}

esp_err_t bsp_spi_add_device(gpio_num_t cs_pin, 
                              uint32_t clock_speed_hz,
                              uint8_t mode,
                              bsp_spi_device_t *out_handle)
{
    if (!spi_initialized) {
        ESP_LOGE(TAG, "SPI bus not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (out_handle == NULL) {
        ESP_LOGE(TAG, "Invalid output handle");
        return ESP_ERR_INVALID_ARG;
    }

    if (mode > 3) {
        ESP_LOGE(TAG, "Invalid SPI mode: %d (must be 0-3)", mode);
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "Adding SPI device (CS=GPIO%d, freq=%lu Hz, mode=%d)", 
             cs_pin, clock_speed_hz, mode);

    spi_device_interface_config_t dev_config = {
        .command_bits = 0,
        .address_bits = 0,
        .dummy_bits = 0,
        .mode = mode,
        .duty_cycle_pos = 128,  // 50% duty cycle
        .cs_ena_pretrans = 0,
        .cs_ena_posttrans = 0,
        .clock_speed_hz = clock_speed_hz,
        .input_delay_ns = 0,
        .spics_io_num = cs_pin,
        .flags = 0,
        .queue_size = 7,  // Max 7 queued transactions
        .pre_cb = NULL,
        .post_cb = NULL
    };

    esp_err_t ret = spi_bus_add_device(BSP_DISPLAY_SPI_HOST, 
                                       &dev_config, 
                                       out_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add device: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "✓ SPI device added (handle=%p)", (void*)*out_handle);
    return ESP_OK;
}

esp_err_t bsp_spi_remove_device(bsp_spi_device_t device)
{
    if (!spi_initialized) {
        ESP_LOGE(TAG, "SPI bus not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (device == NULL) {
        ESP_LOGE(TAG, "Invalid device handle");
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "Removing SPI device (handle=%p)", (void*)device);
    
    esp_err_t ret = spi_bus_remove_device(device);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to remove device: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "✓ SPI device removed");
    return ESP_OK;
}

esp_err_t bsp_spi_acquire(uint32_t timeout_ms)
{
    if (!spi_initialized || spi_mutex == NULL) {
        ESP_LOGE(TAG, "SPI bus not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    TickType_t timeout_ticks = (timeout_ms == UINT32_MAX) 
                               ? portMAX_DELAY 
                               : pdMS_TO_TICKS(timeout_ms);

    uint64_t start_time = esp_timer_get_time();

    BaseType_t result = xSemaphoreTake(spi_mutex, timeout_ticks);
    
    if (result != pdTRUE) {
        stats.mutex_timeouts++;
        ESP_LOGW(TAG, "SPI mutex timeout after %lu ms (holder=%p)", 
                 timeout_ms, (void*)mutex_holder);
        return ESP_ERR_TIMEOUT;
    }

    uint64_t wait_time_ms = (esp_timer_get_time() - start_time) / 1000;
    if (wait_time_ms > stats.max_wait_time_ms) {
        stats.max_wait_time_ms = (uint32_t)wait_time_ms;
    }

    mutex_holder = xTaskGetCurrentTaskHandle();
    
    ESP_LOGD(TAG, "SPI mutex acquired (task=%s, wait=%llu ms)", 
             pcTaskGetName(NULL), wait_time_ms);
    
    return ESP_OK;
}

esp_err_t bsp_spi_release(void)
{
    if (!spi_initialized || spi_mutex == NULL) {
        ESP_LOGE(TAG, "SPI bus not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    TaskHandle_t current_task = xTaskGetCurrentTaskHandle();
    if (mutex_holder != current_task) {
        ESP_LOGE(TAG, "Mutex not held by current task!");
        ESP_LOGE(TAG, "  Holder: %s", mutex_holder ? pcTaskGetName(mutex_holder) : "NULL");
        ESP_LOGE(TAG, "  Current: %s", pcTaskGetName(current_task));
        return ESP_FAIL;
    }

    mutex_holder = NULL;
    
    BaseType_t result = xSemaphoreGive(spi_mutex);
    if (result != pdTRUE) {
        ESP_LOGE(TAG, "Failed to release mutex");
        return ESP_FAIL;
    }

    ESP_LOGD(TAG, "SPI mutex released (task=%s)", pcTaskGetName(NULL));
    return ESP_OK;
}

esp_err_t bsp_spi_transmit(bsp_spi_device_t device, spi_transaction_t *trans)
{
    if (!spi_initialized) {
        ESP_LOGE(TAG, "SPI bus not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (device == NULL || trans == NULL) {
        ESP_LOGE(TAG, "Invalid arguments");
        return ESP_ERR_INVALID_ARG;
    }

    // Verify mutex is held
    TaskHandle_t current_task = xTaskGetCurrentTaskHandle();
    if (mutex_holder != current_task) {
        ESP_LOGE(TAG, "SPI mutex not held! Call bsp_spi_acquire() first");
        return ESP_ERR_INVALID_STATE;
    }

    stats.total_transactions++;

    esp_err_t ret = spi_device_polling_transmit(device, trans);
    
    if (ret != ESP_OK) {
        stats.failed_transactions++;
        ESP_LOGE(TAG, "SPI transmit failed: %s", esp_err_to_name(ret));
        return ret;
    }

    // Update statistics
    if (trans->tx_buffer != NULL) {
        stats.total_bytes_tx += trans->length / 8;  // bits to bytes
    }
    if (trans->rx_buffer != NULL) {
        stats.total_bytes_rx += trans->rxlength / 8;
    }

    ESP_LOGV(TAG, "SPI transaction complete (%d bytes)", trans->length / 8);
    return ESP_OK;
}

esp_err_t bsp_spi_queue_trans(bsp_spi_device_t device, 
                               spi_transaction_t *trans,
                               uint32_t timeout_ms)
{
    if (!spi_initialized) {
        ESP_LOGE(TAG, "SPI bus not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (device == NULL || trans == NULL) {
        ESP_LOGE(TAG, "Invalid arguments");
        return ESP_ERR_INVALID_ARG;
    }

    // Verify mutex is held
    TaskHandle_t current_task = xTaskGetCurrentTaskHandle();
    if (mutex_holder != current_task) {
        ESP_LOGE(TAG, "SPI mutex not held!");
        return ESP_ERR_INVALID_STATE;
    }

    TickType_t timeout_ticks = pdMS_TO_TICKS(timeout_ms);
    
    esp_err_t ret = spi_device_queue_trans(device, trans, timeout_ticks);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to queue transaction: %s", esp_err_to_name(ret));
        return ret;
    }

    stats.total_transactions++;
    
    ESP_LOGV(TAG, "SPI transaction queued");
    return ESP_OK;
}

esp_err_t bsp_spi_get_trans_result(bsp_spi_device_t device,
                                    spi_transaction_t **out_trans,
                                    uint32_t timeout_ms)
{
    if (!spi_initialized) {
        ESP_LOGE(TAG, "SPI bus not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (device == NULL) {
        ESP_LOGE(TAG, "Invalid device handle");
        return ESP_ERR_INVALID_ARG;
    }

    TickType_t timeout_ticks = pdMS_TO_TICKS(timeout_ms);
    
    esp_err_t ret = spi_device_get_trans_result(device, out_trans, timeout_ticks);
    
    if (ret != ESP_OK) {
        if (ret == ESP_ERR_TIMEOUT) {
            ESP_LOGW(TAG, "Timeout waiting for transaction result");
        } else {
            ESP_LOGE(TAG, "Failed to get transaction result: %s", esp_err_to_name(ret));
        }
        return ret;
    }

    // Update statistics if transaction succeeded
    if (out_trans != NULL && *out_trans != NULL) {
        spi_transaction_t *trans = *out_trans;
        if (trans->tx_buffer != NULL) {
            stats.total_bytes_tx += trans->length / 8;
        }
        if (trans->rx_buffer != NULL) {
            stats.total_bytes_rx += trans->rxlength / 8;
        }
    }

    ESP_LOGV(TAG, "SPI transaction result retrieved");
    return ESP_OK;
}

esp_err_t bsp_spi_get_stats(bsp_spi_stats_t *out_stats)
{
    if (out_stats == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    memcpy(out_stats, &stats, sizeof(bsp_spi_stats_t));
    return ESP_OK;
}

esp_err_t bsp_spi_reset_stats(void)
{
    memset(&stats, 0, sizeof(bsp_spi_stats_t));
    ESP_LOGI(TAG, "SPI statistics reset");
    return ESP_OK;
}

bool bsp_spi_is_acquired(void)
{
    if (!spi_initialized || spi_mutex == NULL) {
        return false;
    }

    TaskHandle_t current_task = xTaskGetCurrentTaskHandle();
    return (mutex_holder == current_task);
}
