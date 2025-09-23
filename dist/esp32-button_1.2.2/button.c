#include <stdio.h>
#include <string.h>
#include <limits.h>

#include <esp_log.h>

#include "toggle.h"
#include "button.h"
#include "port.h"


typedef enum {
        button_timer_mode_idle = 0,
        button_timer_mode_long_press,
        button_timer_mode_repeat_window,
} button_timer_mode_t;

typedef struct _button {
        gpio_num_t gpio_num;
        button_config_t config;
        button_callback_fn callback;
        void* context;

        uint16_t press_count;
        TimerHandle_t event_timer;
        button_timer_mode_t timer_mode;
} button_t;

static SemaphoreHandle_t buttons_lock = NULL;
static button_t *registered_buttons[GPIO_NUM_MAX];
static button_t button_pool[GPIO_NUM_MAX];
static StaticTimer_t button_timer_buffers[GPIO_NUM_MAX];
static bool button_claimed[GPIO_NUM_MAX];
static TickType_t button_ms_to_ticks(uint16_t duration_ms) {
        if (!duration_ms)
                return 0;

        TickType_t ticks = pdMS_TO_TICKS(duration_ms);
        return ticks ? ticks : 1;
}
static const char *TAG = "button";

static void button_fire_event(button_t *button) {
        if (!button->press_count)
                return;

        button_event_t event = button_event_single_press;

        switch (button->press_count) {
        case 1: event = button_event_single_press; break;
        case 2: event = button_event_double_press; break;
        default: event = button_event_tripple_press; break;
        }

        button->callback(event, button->context);
        button->press_count = 0;
}

static void button_toggle_callback(bool high, void *context) {
        if (!context)
                return;

        button_t *button = (button_t*) context;
        const bool pressed = (high == (button->config.active_level == button_active_high));

        if (pressed) {
                if (button->press_count < button->config.max_repeat_presses) {
                        button->press_count++;
                } else {
                        button->press_count = button->config.max_repeat_presses;
                }

                if (button->event_timer && button->timer_mode == button_timer_mode_repeat_window) {
                        if (xTimerIsTimerActive(button->event_timer)) {
                                xTimerStop(button->event_timer, 0);
                        }
                        button->timer_mode = button_timer_mode_idle;
                }

                if (button->event_timer && button->config.long_press_time && button->press_count == 1) {
                        button->timer_mode = button_timer_mode_long_press;
                        TickType_t ticks = button_ms_to_ticks(button->config.long_press_time);
                        if (ticks) {
                                xTimerChangePeriod(button->event_timer, ticks, 0);
                        }
                }
        } else {
                if (!button->press_count)
                        return;

                if (button->event_timer && button->timer_mode == button_timer_mode_long_press
                    && xTimerIsTimerActive(button->event_timer)) {
                        xTimerStop(button->event_timer, 0);
                        button->timer_mode = button_timer_mode_idle;
                }

                const bool reached_limit = button->press_count >= button->config.max_repeat_presses;
                const bool repeat_disabled = (!button->config.repeat_press_timeout
                                               || button->config.max_repeat_presses <= 1);

                if (reached_limit || repeat_disabled || !button->event_timer) {
                        if (button->event_timer && xTimerIsTimerActive(button->event_timer)) {
                                xTimerStop(button->event_timer, 0);
                        }
                        button->timer_mode = button_timer_mode_idle;
                        button_fire_event(button);
                } else {
                        TickType_t ticks = button_ms_to_ticks(button->config.repeat_press_timeout);
                        if (!ticks) {
                                if (button->event_timer && xTimerIsTimerActive(button->event_timer)) {
                                        xTimerStop(button->event_timer, 0);
                                }
                                button->timer_mode = button_timer_mode_idle;
                                button_fire_event(button);
                        } else {
                                button->timer_mode = button_timer_mode_repeat_window;
                                xTimerChangePeriod(button->event_timer, ticks, 0);
                        }
                }
        }
}

static void button_event_timer_callback(TimerHandle_t timer) {
        button_t *button = (button_t*) pvTimerGetTimerID(timer);
        if (!button)
                return;

        switch (button->timer_mode) {
        case button_timer_mode_long_press:
                button->timer_mode = button_timer_mode_idle;
                button->callback(button_event_long_press, button->context);
                button->press_count = 0;
                break;
        case button_timer_mode_repeat_window:
                button->timer_mode = button_timer_mode_idle;
                button_fire_event(button);
                break;
        default:
                break;
        }
}

static void button_reset(button_t *button) {
        if (!button)
                return;

        if (button->event_timer) {
                xTimerStop(button->event_timer, 0);
                xTimerDelete(button->event_timer, 0);
                button->event_timer = NULL;
        }

        button->timer_mode = button_timer_mode_idle;
        button->press_count = 0;
        memset(button, 0, sizeof(*button));
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

        if (!callback) {
                ESP_LOGE(TAG, "Callback must not be NULL for GPIO %d", (int) gpio_num);
                return -6;
        }

        if (!buttons_lock && buttons_init() != 0) {
                return -7;
        }

        button_config_t normalized = config;
        if (!normalized.max_repeat_presses) {
                normalized.max_repeat_presses = 1;
        }
        if (normalized.max_repeat_presses > UINT16_MAX) {
                normalized.max_repeat_presses = UINT16_MAX;
        }

        const size_t index = (size_t) gpio_num;
        button_t *button = &button_pool[index];

        xSemaphoreTake(buttons_lock, portMAX_DELAY);
        if (button_claimed[index]) {
                xSemaphoreGive(buttons_lock);
                return -1;
        }
        button_claimed[index] = true;
        xSemaphoreGive(buttons_lock);

        button_reset(button);

        button->gpio_num = gpio_num;
        button->config = normalized;
        button->callback = callback;
        button->context = context;
        button->timer_mode = button_timer_mode_idle;

        const bool needs_timer = (normalized.long_press_time > 0)
                || (normalized.max_repeat_presses > 1 && normalized.repeat_press_timeout > 0);

        int result = -4;
        bool toggle_ready = false;

        if (needs_timer) {
                button->event_timer = xTimerCreateStatic(
                        "Button Event Timer",
                        1,
                        pdFALSE,
                        button,
                        button_event_timer_callback,
                        &button_timer_buffers[index]
                );
                if (!button->event_timer) {
                        ESP_LOGE(TAG, "Failed to create timer for button on GPIO %d", (int) gpio_num);
                        result = -2;
                        goto fail;
                }
        }

        result = toggle_create(gpio_num, button_toggle_callback, button);
        if (result) {
                result = (result == -1) ? -1 : -4;
                goto fail;
        }
        toggle_ready = true;

        if (normalized.active_level == button_active_low) {
                my_gpio_pullup(button->gpio_num);
        } else {
                my_gpio_pulldown(button->gpio_num);
        }

        toggle_sync_state(button->gpio_num);

        xSemaphoreTake(buttons_lock, portMAX_DELAY);
        registered_buttons[index] = button;
        button_claimed[index] = true;
        xSemaphoreGive(buttons_lock);

        return 0;

fail:
        if (toggle_ready) {
                toggle_delete(button->gpio_num);
        }

        button_reset(button);

        xSemaphoreTake(buttons_lock, portMAX_DELAY);
        registered_buttons[index] = NULL;
        button_claimed[index] = false;
        xSemaphoreGive(buttons_lock);

        return result;
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
        if (!button) {
                xSemaphoreGive(buttons_lock);
                return;
        }

        registered_buttons[index] = NULL;

        toggle_delete(button->gpio_num);
        button_reset(button);

        button_claimed[index] = false;

        xSemaphoreGive(buttons_lock);
}
