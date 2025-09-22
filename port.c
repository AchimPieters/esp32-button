#include "port.h"

#include <driver/gpio.h>
#include <esp_err.h>
#include <esp_log.h>
#include <stdbool.h>

static const char *TAG = "button_port";

static void log_gpio_error(gpio_num_t gpio, const char *action, esp_err_t err) {
        ESP_LOGE(TAG, "%s failed for GPIO %d: %s", action, (int) gpio, esp_err_to_name(err));
}

static bool validate_gpio(gpio_num_t gpio) {
        if (!GPIO_IS_VALID_GPIO(gpio)) {
                ESP_LOGE(TAG, "Invalid GPIO number: %d", (int) gpio);
                return false;
        }

        return true;
}

// Function to configure GPIO as input
void my_gpio_enable(gpio_num_t gpio) {
        if (!validate_gpio(gpio)) {
                return;
        }

        esp_err_t err = gpio_reset_pin(gpio);
        if (err != ESP_OK) {
                log_gpio_error(gpio, "gpio_reset_pin", err);
                return;
        }

        gpio_config_t io_conf = {
                .pin_bit_mask = 1ULL << (uint32_t) gpio,
                .mode = GPIO_MODE_INPUT,
                .pull_up_en = false,
                .pull_down_en = false,
                .intr_type = GPIO_INTR_DISABLE,
        };

        err = gpio_config(&io_conf);
        if (err != ESP_OK) {
                log_gpio_error(gpio, "gpio_config", err);
        }
}

// Function to set GPIO pullup
void my_gpio_pullup(gpio_num_t gpio) {
        if (!validate_gpio(gpio)) {
                return;
        }

        esp_err_t err = gpio_pullup_en(gpio);
        if (err != ESP_OK) {
                log_gpio_error(gpio, "gpio_pullup_en", err);
        }

        err = gpio_pulldown_dis(gpio);
        if (err != ESP_OK) {
                log_gpio_error(gpio, "gpio_pulldown_dis", err);
        }
}

// Function to set GPIO pulldown
void my_gpio_pulldown(gpio_num_t gpio) {
        if (!validate_gpio(gpio)) {
                return;
        }

        esp_err_t err = gpio_pulldown_en(gpio);
        if (err != ESP_OK) {
                log_gpio_error(gpio, "gpio_pulldown_en", err);
        }

        err = gpio_pullup_dis(gpio);
        if (err != ESP_OK) {
                log_gpio_error(gpio, "gpio_pullup_dis", err);
        }
}

// Function to read GPIO level
uint8_t my_gpio_read(gpio_num_t gpio) {
        if (!validate_gpio(gpio)) {
                return 0;
        }

        return (uint8_t) gpio_get_level(gpio);
}
