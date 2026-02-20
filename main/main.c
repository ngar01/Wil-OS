/**
 * @file main.c
 * @brief Wil-OS Entry Point - System Initialization and Bootstrap
 * 
 * This file orchestrates the complete system startup sequence:
 *   1. Hardware initialization (BSP)
 *   2. Driver initialization (HAL)
 *   3. Service startup (Middleware)
 *   4. Framework launch (App Manager)
 *   5. Launch default application (Launcher)
 * 
 * @author Wil-OS Development Team
 * @version 1.0.0
 * @date 2024
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "nvs_flash.h"
#include "esp_err.h"

// Wil-OS Versioning
#define WIL_OS_VERSION_MAJOR 1
#define WIL_OS_VERSION_MINOR 0
#define WIL_OS_VERSION_PATCH 0
// Wil-OS component headers (to be implemented)
#include "bsp.h"
// #include "bsp_common.h"
// #include "hal_display.h"
// #include "hal_storage.h"
// #include "hal_input.h"
// #include "gui_core.h"
// #include "fs_manager.h"
// #include "input_manager.h"
// #include "app_manager.h"
// #include "system_monitor.h"

static const char *TAG = "MAIN";

/**
 * @brief Boot time measurement
 * Global variable to track system startup performance
 */
static uint64_t boot_start_us = 0;

/**
 * @brief Initialize Non-Volatile Storage
 * 
 * NVS is used for:
 *   - Wi-Fi credentials
 *   - OTA server configuration
 *   - User preferences
 *   - Application settings
 * 
 * @return ESP_OK on success, error code otherwise
 */
static esp_err_t init_nvs(void)
{
    ESP_LOGI(TAG, "Initializing NVS...");
    
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated or format changed, erase and retry
        ESP_LOGW(TAG, "NVS partition needs formatting");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "✓ NVS initialized");
    }
    
    return ret;
}

/**
 * @brief Initialize Board Support Package
 * 
 * Configures:
 *   - GPIO pin mapping
 *   - SPI bus (shared by display and SD card)
 *   - DMA channels
 *   - Power management
 * 
 * @return ESP_OK on success
 */
static esp_err_t init_bsp(void)
{
    ESP_LOGI(TAG, "Initializing BSP...");
    
    esp_err_t ret = bsp_init(NULL); 
    ESP_ERROR_CHECK(ret);
    
    ESP_LOGI(TAG, "✓ BSP initialized (GPIO, SPI, DMA configured)");
    return ESP_OK;
}

/**
 * @brief Initialize Hardware Abstraction Layer
 * 
 * Initializes device drivers:
 *   - Display (ST7735)
 *   - Storage (SD Card via SPI)
 *   - Input (4x4 Keypad)
 * 
 * @return ESP_OK on success
 */
static esp_err_t init_hal(void)
{
    ESP_LOGI(TAG, "Initializing HAL drivers...");
    
    // TODO: Implement in Phase 1
    // Display driver
    // esp_err_t ret = hal_display_init(NULL); // Use default config
    // ESP_ERROR_CHECK(ret);
    
    // Storage driver (SD card)
    // ret = hal_storage_init(NULL);
    // ESP_ERROR_CHECK(ret);
    
    // Input driver (keypad)
    // ret = hal_input_init(NULL);
    // ESP_ERROR_CHECK(ret);
    
    ESP_LOGI(TAG, "✓ HAL initialized (Display, Storage, Input ready)");
    return ESP_OK;
}

/**
 * @brief Initialize Middleware Services
 * 
 * Starts high-level services:
 *   - GUI engine (rendering, widgets)
 *   - File system manager (LittleFS)
 *   - Input manager (event queue)
 * 
 * @return ESP_OK on success
 */
static esp_err_t init_services(void)
{
    ESP_LOGI(TAG, "Initializing services...");
    
    // TODO: Implement in Phase 1
    // GUI engine
    // esp_err_t ret = gui_init();
    // ESP_ERROR_CHECK(ret);
    
    // File system
    // ret = fs_manager_init();
    // ESP_ERROR_CHECK(ret);
    
    // Input manager
    // ret = input_manager_init();
    // ESP_ERROR_CHECK(ret);
    
    ESP_LOGI(TAG, "✓ Services initialized");
    return ESP_OK;
}

/**
 * @brief Initialize Application Framework
 * 
 * Configures:
 *   - App Manager (lifecycle, loading)
 *   - Event Bus (inter-app communication)
 *   - System Monitor (watchdog, heap tracking)
 * 
 * @return ESP_OK on success
 */
static esp_err_t init_framework(void)
{
    ESP_LOGI(TAG, "Initializing framework...");
    
    // TODO: Implement in Phase 1
    // App Manager
    // esp_err_t ret = app_manager_init();
    // ESP_ERROR_CHECK(ret);
    
    // System Monitor (TWDT, heap, telemetry)
    // ret = system_monitor_start();
    // ESP_ERROR_CHECK(ret);
    
    ESP_LOGI(TAG, "✓ Framework initialized");
    return ESP_OK;
}

/**
 * @brief Print boot summary
 * 
 * Displays:
 *   - Total boot time
 *   - Available memory (IRAM, PSRAM)
 *   - Chip information
 */
static void print_boot_summary(void)
{
    uint64_t boot_time_ms = (esp_timer_get_time() - boot_start_us) / 1000;
    
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "  Wil-OS Boot Complete");
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "Boot time: %llu ms", boot_time_ms);
    ESP_LOGI(TAG, "Target: ESP32-S3");
    ESP_LOGI(TAG, "CPU Freq: %d MHz", CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ);
    ESP_LOGI(TAG, "Free heap: %lu bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "Free PSRAM: %lu bytes", esp_get_free_internal_heap_size());
    ESP_LOGI(TAG, "Minimum free heap: %lu bytes", esp_get_minimum_free_heap_size());
    ESP_LOGI(TAG, "========================================");
    
    // Performance check
    if (boot_time_ms > 2000) {
        ESP_LOGW(TAG, "⚠ Boot time exceeds 2s target!");
    } else {
        ESP_LOGI(TAG, "✓ Boot time within target");
    }
}

/**
 * @brief Application entry point (ESP-IDF standard)
 * 
 * This function is called by the bootloader after basic hardware init.
 * It runs once in app_main task, then the task deletes itself.
 * 
 * Initialization sequence:
 *   1. Start boot timer
 *   2. NVS (configuration storage)
 *   3. BSP (low-level hardware)
 *   4. HAL (device drivers)
 *   5. Services (middleware)
 *   6. Framework (app orchestration)
 *   7. Launch first app (Launcher menu)
 */
void app_main(void)
{
    boot_start_us = esp_timer_get_time();
    
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "  Wil-OS v%d.%d.%d", 
             WIL_OS_VERSION_MAJOR, 
             WIL_OS_VERSION_MINOR, 
             WIL_OS_VERSION_PATCH);
    ESP_LOGI(TAG, "  Professional-Grade RTOS for ESP32-S3");
    ESP_LOGI(TAG, "========================================");
    
    // Phase 1: Core system initialization
    ESP_ERROR_CHECK(init_nvs());
    ESP_ERROR_CHECK(init_bsp());
    ESP_ERROR_CHECK(init_hal());
    
    // Phase 2: Service layer
    ESP_ERROR_CHECK(init_services());
    
    // Phase 3: Application framework
    ESP_ERROR_CHECK(init_framework());
    
    // Phase 4: Display boot summary
    print_boot_summary();
    
    // Phase 5: Launch default application
    ESP_LOGI(TAG, "Launching Launcher app...");
    // TODO: Implement in Phase 1
    // app_manager_launch("launcher");
    
    ESP_LOGI(TAG, "System ready. Entering main loop...");
    
    // Note: app_main task will be deleted by ESP-IDF after this function returns
    // The system continues running via FreeRTOS tasks created during init
}

/**
 * @brief Error handler hook
 * 
 * Called when ESP_ERROR_CHECK fails (assertion)
 * Override default behavior to add custom logging/recovery
 */
void esp_error_check_failed_hook(esp_err_t err, const char *file, int line)
{
    ESP_LOGE(TAG, "ERROR CHECK FAILED: 0x%x at %s:%d", err, file, line);
    
    // Log to SD card for post-mortem analysis
    // TODO: Add persistent error logging
    
    // Default behavior: abort and trigger core dump
}
