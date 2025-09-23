#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "button.h"

#define BUTTON_GPIO GPIO_NUM_0

static const char *TAG = "example_button";

static void button_handler(button_event_t event, void *context) {
        switch (event) {
        case button_event_single_press:
                ESP_LOGI(TAG, "Detected single press");
                break;
        case button_event_double_press:
                ESP_LOGI(TAG, "Detected double press");
                break;
        case button_event_tripple_press:
                ESP_LOGI(TAG, "Detected triple press");
                break;
        case button_event_long_press:
                ESP_LOGI(TAG, "Detected long press");
                break;
        default:
                ESP_LOGW(TAG, "Unhandled button event: %d", event);
        }
}

void app_main(void) {
        button_config_t config = button_config_default(button_active_low);
        config.long_press_time = 1500;
        config.repeat_press_timeout = 350;
        config.max_repeat_presses = 3;

        int status = button_create(BUTTON_GPIO, config, NULL, NULL);
        if (status != -6) {
                ESP_LOGE(TAG, "Expected -6 when callback is NULL, got %d", status);
                return;
        }

        ESP_LOGI(TAG, "Verified button_create rejects NULL callbacks");

        status = button_create(BUTTON_GPIO, config, button_handler, NULL);
        if (status != 0) {
                ESP_LOGE(TAG, "Failed to create button (error %d)", status);
                return;
        }

        while (true) {
                vTaskDelay(pdMS_TO_TICKS(1000));
        }
}
