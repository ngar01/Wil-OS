/**
 * @file bsp_gpio.c
 * @brief GPIO Initialization and Management Implementation
 * 
 * @author Wil-OS Team
 * @version 1.0.0
 */

#include "bsp_gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"

static const char *TAG = BSP_GPIO_TAG;

// PWM configuration for backlight
#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_8_BIT    // 0-255
#define LEDC_FREQUENCY          5000                // 5 kHz

static bool backlight_initialized = false;

esp_err_t bsp_gpio_init(void)
{
    esp_err_t ret;

    ESP_LOGI(TAG, "Configuring GPIO pins...");

    // ========================================================================
    // Display Control Pins
    // ========================================================================

    // DC (Data/Command) - Output
    gpio_config_t io_conf_dc = {
        .pin_bit_mask = (1ULL << BSP_DISPLAY_DC_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    ret = gpio_config(&io_conf_dc);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure DC pin: %s", esp_err_to_name(ret));
        return ret;
    }
    gpio_set_level(BSP_DISPLAY_DC_PIN, 0); // Default to command mode

    // RST (Reset) - Output, initially high
    gpio_config_t io_conf_rst = {
        .pin_bit_mask = (1ULL << BSP_DISPLAY_RST_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    ret = gpio_config(&io_conf_rst);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure RST pin: %s", esp_err_to_name(ret));
        return ret;
    }
    gpio_set_level(BSP_DISPLAY_RST_PIN, 1); // Inactive (reset is active low)

    // CS (Chip Select) - Output, initially high (inactive)
    gpio_config_t io_conf_display_cs = {
        .pin_bit_mask = (1ULL << BSP_DISPLAY_CS_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    ret = gpio_config(&io_conf_display_cs);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure Display CS pin: %s", esp_err_to_name(ret));
        return ret;
    }
    gpio_set_level(BSP_DISPLAY_CS_PIN, 1); // Inactive

    ESP_LOGI(TAG, "✓ Display pins configured (DC=%d, RST=%d, CS=%d)", 
             BSP_DISPLAY_DC_PIN, BSP_DISPLAY_RST_PIN, BSP_DISPLAY_CS_PIN);

    // ========================================================================
    // SD Card Control Pin
    // ========================================================================

    gpio_config_t io_conf_sd_cs = {
        .pin_bit_mask = (1ULL << BSP_SDCARD_CS_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    ret = gpio_config(&io_conf_sd_cs);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure SD CS pin: %s", esp_err_to_name(ret));
        return ret;
    }
    gpio_set_level(BSP_SDCARD_CS_PIN, 1); // Inactive

    ESP_LOGI(TAG, "✓ SD Card CS configured (GPIO%d)", BSP_SDCARD_CS_PIN);

    // ========================================================================
    // Keypad Matrix Pins
    // ========================================================================

    // Row pins - Outputs (driven by ESP32)
    uint64_t row_mask = (1ULL << BSP_KEYPAD_ROW1_PIN) |
                        (1ULL << BSP_KEYPAD_ROW2_PIN) |
                        (1ULL << BSP_KEYPAD_ROW3_PIN) |
                        (1ULL << BSP_KEYPAD_ROW4_PIN);
    
    gpio_config_t io_conf_rows = {
        .pin_bit_mask = row_mask,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    ret = gpio_config(&io_conf_rows);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure keypad row pins: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Set all rows high initially
    gpio_set_level(BSP_KEYPAD_ROW1_PIN, 1);
    gpio_set_level(BSP_KEYPAD_ROW2_PIN, 1);
    gpio_set_level(BSP_KEYPAD_ROW3_PIN, 1);
    gpio_set_level(BSP_KEYPAD_ROW4_PIN, 1);

    // Column pins - Inputs with pull-ups
    uint64_t col_mask = (1ULL << BSP_KEYPAD_COL1_PIN) |
                        (1ULL << BSP_KEYPAD_COL2_PIN) |
                        (1ULL << BSP_KEYPAD_COL3_PIN) |
                        (1ULL << BSP_KEYPAD_COL4_PIN);
    
    gpio_config_t io_conf_cols = {
        .pin_bit_mask = col_mask,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,  // Pull-up resistors
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    ret = gpio_config(&io_conf_cols);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure keypad col pins: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "✓ Keypad matrix configured (4 rows, 4 cols)");

    // ========================================================================
    // Backlight PWM (using LEDC peripheral)
    // ========================================================================

    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = LEDC_FREQUENCY,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ret = ledc_timer_config(&ledc_timer);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure LEDC timer: %s", esp_err_to_name(ret));
        return ret;
    }

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = BSP_DISPLAY_BL_PIN,
        .duty           = 0, // Start with backlight off
        .hpoint         = 0
    };
    ret = ledc_channel_config(&ledc_channel);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure LEDC channel: %s", esp_err_to_name(ret));
        return ret;
    }

    backlight_initialized = true;
    
    // Set backlight to 80% by default
    bsp_gpio_set_backlight(80);
    
    ESP_LOGI(TAG, "✓ Backlight PWM configured (GPIO%d @ %d Hz)", 
             BSP_DISPLAY_BL_PIN, LEDC_FREQUENCY);

    ESP_LOGI(TAG, "GPIO initialization complete");
    return ESP_OK;
}

esp_err_t bsp_gpio_set_high(gpio_num_t pin)
{
    if (!BSP_GPIO_IS_VALID(pin)) {
        return ESP_ERR_INVALID_ARG;
    }
    return gpio_set_level(pin, 1);
}

esp_err_t bsp_gpio_set_low(gpio_num_t pin)
{
    if (!BSP_GPIO_IS_VALID(pin)) {
        return ESP_ERR_INVALID_ARG;
    }
    return gpio_set_level(pin, 0);
}

int bsp_gpio_get_level(gpio_num_t pin)
{
    if (!BSP_GPIO_IS_VALID(pin)) {
        return -1;
    }
    return gpio_get_level(pin);
}

esp_err_t bsp_gpio_toggle(gpio_num_t pin)
{
    if (!BSP_GPIO_IS_VALID(pin)) {
        return ESP_ERR_INVALID_ARG;
    }
    
    int current_level = gpio_get_level(pin);
    return gpio_set_level(pin, !current_level);
}

esp_err_t bsp_gpio_set_backlight(uint8_t brightness)
{
    if (!backlight_initialized) {
        ESP_LOGE(TAG, "Backlight not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (brightness > 100) {
        ESP_LOGW(TAG, "Brightness clamped to 100%%");
        brightness = 100;
    }

    // Convert 0-100% to 0-255 duty cycle
    uint32_t duty = (brightness * 255) / 100;

    esp_err_t ret = ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, duty);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set backlight duty: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to update backlight: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGD(TAG, "Backlight set to %d%% (duty=%lu)", brightness, duty);
    return ESP_OK;
}
