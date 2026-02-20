/**
 * @file bsp_init.c
 * @brief Board Support Package Main Initialization
 * 
 * @author Wil-OS Team
 * @version 1.0.0
 */

#include "bsp_common.h"
#include "bsp_gpio.h"
#include "bsp_spi.h"
#include "esp_log.h"
#include "esp_system.h"

static const char *TAG = BSP_TAG;
static bool bsp_initialized = false;

/**
 * @brief Print BSP information
 */
static void bsp_print_info(void)
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "  Board Support Package v%d.%d.%d", 
             BSP_HARDWARE_VERSION_MAJOR,
             BSP_HARDWARE_VERSION_MINOR,
             BSP_HARDWARE_VERSION_PATCH);
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "Display: ST7735 %dx%d @ GPIO%d (CS)", 
             BSP_DISPLAY_WIDTH, BSP_DISPLAY_HEIGHT, BSP_DISPLAY_CS_PIN);
    ESP_LOGI(TAG, "SD Card: SPI mode @ GPIO%d (CS)", BSP_SDCARD_CS_PIN);
    ESP_LOGI(TAG, "Keypad: 4x4 matrix (GPIO%d-GPIO%d rows)", 
             BSP_KEYPAD_ROW1_PIN, BSP_KEYPAD_ROW4_PIN);
    ESP_LOGI(TAG, "SPI Bus: %d MHz DMA enabled", 
             BSP_DISPLAY_SPI_FREQ_HZ / 1000000);
    ESP_LOGI(TAG, "========================================");
}

esp_err_t bsp_init(const bsp_config_t *config)
{
    if (bsp_initialized) {
        ESP_LOGW(TAG, "BSP already initialized");
        return ESP_ERR_INVALID_STATE;
    }

    // Use default config if none provided
    bsp_config_t default_config = BSP_CONFIG_DEFAULT();
    if (config == NULL) {
        config = &default_config;
    }

    ESP_LOGI(TAG, "Initializing Board Support Package...");
    
    bsp_print_info();

    esp_err_t ret;

    // Step 1: Initialize GPIO
    if (config->init_gpio) {
        ESP_LOGI(TAG, "Initializing GPIO...");
        ret = bsp_gpio_init();
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "GPIO init failed: %s", esp_err_to_name(ret));
            return ret;
        }
        ESP_LOGI(TAG, "✓ GPIO initialized");
    }

    // Step 2: Initialize SPI bus
    if (config->init_spi) {
        ESP_LOGI(TAG, "Initializing SPI bus...");
        ret = bsp_spi_init();
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "SPI init failed: %s", esp_err_to_name(ret));
            return ret;
        }
        ESP_LOGI(TAG, "✓ SPI bus initialized with mutex protection");
    }

    // Step 3: Power management (optional)
    if (config->init_power) {
        ESP_LOGI(TAG, "Initializing power management...");
        // TODO: Implement power management
        ESP_LOGW(TAG, "Power management not yet implemented");
    }

    bsp_initialized = true;
    ESP_LOGI(TAG, "✓ BSP initialization complete");

    return ESP_OK;
}

esp_err_t bsp_deinit(void)
{
    if (!bsp_initialized) {
        ESP_LOGW(TAG, "BSP not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "De-initializing BSP...");

    // Deinitialize in reverse order
    esp_err_t ret = bsp_spi_deinit();
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "SPI deinit failed: %s", esp_err_to_name(ret));
    }

    // GPIO deinit not necessary (will be reconfigured on next init)

    bsp_initialized = false;
    ESP_LOGI(TAG, "✓ BSP deinitialized");

    return ESP_OK;
}

bool bsp_is_initialized(void)
{
    return bsp_initialized;
}
