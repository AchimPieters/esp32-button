#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <esp_log.h>

#include "toggle.h"
#include "button.h"
#include "port.h"


typedef struct _button {
        gpio_num_t gpio_num;
        button_config_t config;
        button_callback_fn callback;
        void* context;

        uint8_t press_count;
        TimerHandle_t long_press_timer;
        TimerHandle_t repeat_press_timeout_timer;
} button_t;

static SemaphoreHandle_t buttons_lock = NULL;
static button_t *registered_buttons[GPIO_NUM_MAX];
static const char *TAG = "button";

static void button_fire_event(button_t *button) {
        button_event_t event = button_event_single_press;

        switch (button->press_count) {
        case 1: event = button_event_single_press; break;
        case 2: event = button_event_double_press; break;
        case 3: event = button_event_tripple_press; break;
        }

        button->callback(event, button->context);
        button->press_count = 0;
}

static void button_toggle_callback(bool high, void *context) {
        if (!context)
                return;

        button_t *button = (button_t*) context;
        if (high == (button->config.active_level == button_active_high)) {
                // pressed
                button->press_count++;
                if (button->long_press_timer && button->press_count == 1) {
                        xTimerReset(button->long_press_timer, 1);
                }
        } else {
                // released
                if (!button->press_count)
                        return;

                if (button->long_press_timer
                    && xTimerIsTimerActive(button->long_press_timer)) {
                        xTimerStop(button->long_press_timer, 1);
                }

                if (button->press_count >= button->config.max_repeat_presses
                    || !button->config.repeat_press_timeout) {
                        if (button->repeat_press_timeout_timer
                            && xTimerIsTimerActive(button->repeat_press_timeout_timer)) {
                                xTimerStop(button->repeat_press_timeout_timer, 1);
                        }

                        button_fire_event(button);
                } else {
                        if (button->repeat_press_timeout_timer) {
                                xTimerReset(button->repeat_press_timeout_timer, 1);
                        }
                }
        }
}

static void button_long_press_timer_callback(TimerHandle_t timer) {
        button_t *button = (button_t*) pvTimerGetTimerID(timer);

        button->callback(button_event_long_press, button->context);
        button->press_count = 0;
}

static void button_repeat_press_timeout_timer_callback(TimerHandle_t timer) {
        button_t *button = (button_t*) pvTimerGetTimerID(timer);

        button_fire_event(button);
}

static void button_free(button_t *button) {
        if (button->long_press_timer) {
                xTimerStop(button->long_press_timer, 1);
                xTimerDelete(button->long_press_timer, 1);
        }

        if (button->repeat_press_timeout_timer) {
                xTimerStop(button->repeat_press_timeout_timer, 1);
                xTimerDelete(button->repeat_press_timeout_timer, 1);
        }

        free(button);
}

static int buttons_init() {
        if (!buttons_lock) {
                buttons_lock = xSemaphoreCreateMutex();
                if (!buttons_lock) {
                        ESP_LOGE(TAG, "Failed to create button lock");
                        return -1;
                }
        }

        return 0;
}

// Check if the button with the given GPIO already exists
int button_create(const gpio_num_t gpio_num,
                  button_config_t config,
                  button_callback_fn callback,
                  void* context)
{
        if (!GPIO_IS_VALID_GPIO(gpio_num)) {
                ESP_LOGE(TAG, "Invalid GPIO number: %d", (int) gpio_num);
                return -5;
        }

        if (!buttons_lock && buttons_init() != 0) {
                return -7;
        }

        const size_t index = (size_t) gpio_num;

        xSemaphoreTake(buttons_lock, portMAX_DELAY);
        bool exists = registered_buttons[index] != NULL;
        xSemaphoreGive(buttons_lock);

        if (exists)
                return -1;

        // Create a new button
        button_t *button = malloc(sizeof(button_t));
        if (!button) {
                ESP_LOGE(TAG, "Failed to allocate memory for button on GPIO %d", (int) gpio_num);
                return -6;
        }

        memset(button, 0, sizeof(*button));
        button->gpio_num = gpio_num;
        button->config = config;
        button->callback = callback;
        button->context = context;
        // Create long press timer if needed
        if (config.long_press_time) {
                button->long_press_timer = xTimerCreate(
                        "Button Long Press Timer", pdMS_TO_TICKS(config.long_press_time),
                        pdFALSE, button, button_long_press_timer_callback
                        );
                if (!button->long_press_timer) {
                        button_free(button);
                        return -2;
                }
        }
        // Create repeat press timeout timer if needed
        if (config.max_repeat_presses > 1 && config.repeat_press_timeout) {
                button->repeat_press_timeout_timer = xTimerCreate(
                        "Button Repeat Press Timeout Timer", pdMS_TO_TICKS(config.repeat_press_timeout),
                        pdFALSE, button, button_repeat_press_timeout_timer_callback
                        );
                if (!button->repeat_press_timeout_timer) {
                        button_free(button);
                        return -3;
                }
        }

        // Initialize the toggle
        int r = toggle_create(gpio_num, button_toggle_callback, button);
        if (r) {
                button_free(button);
                return (r == -1) ? -1 : -4;
        }

        if (config.active_level == button_active_low) {
                my_gpio_pullup(button->gpio_num);
        } else {
                my_gpio_pulldown(button->gpio_num);
        }

        toggle_sync_state(button->gpio_num);

        xSemaphoreTake(buttons_lock, portMAX_DELAY);

        if (registered_buttons[index]) {
                xSemaphoreGive(buttons_lock);
                toggle_delete(button->gpio_num);
                button_free(button);
                return -1;
        }

        registered_buttons[index] = button;

        xSemaphoreGive(buttons_lock);

        return 0;
}

void button_destroy(const gpio_num_t gpio_num) {
        if (!GPIO_IS_VALID_GPIO(gpio_num)) {
                ESP_LOGE(TAG, "Invalid GPIO number: %d", (int) gpio_num);
                return;
        }

        if (!buttons_lock && buttons_init() != 0) {
                return;
        }

        button_t *button = NULL;
        const size_t index = (size_t) gpio_num;

        xSemaphoreTake(buttons_lock, portMAX_DELAY);

        button = registered_buttons[index];
        registered_buttons[index] = NULL;

        xSemaphoreGive(buttons_lock);

        if (!button)
                return;

        toggle_delete(button->gpio_num);
        button_free(button);
}
