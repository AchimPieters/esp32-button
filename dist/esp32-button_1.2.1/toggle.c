#include <stdlib.h>
#include <string.h>

#include <stdio.h>

#include <esp_attr.h>
#include <esp_err.h>
#include <esp_intr_alloc.h>
#include <esp_log.h>

#include "toggle.h"
#include "port.h"


typedef struct _toggle {
        gpio_num_t gpio_num;
        toggle_callback_fn callback;
        void* context;

        bool last_high;
        bool debounce_timer_armed;
        TimerHandle_t debounce_timer;
} toggle_t;


#define TOGGLE_DEBOUNCE_MS 10


static SemaphoreHandle_t toggles_lock = NULL;
static toggle_t toggle_pool[GPIO_NUM_MAX];
static StaticTimer_t toggle_timer_buffers[GPIO_NUM_MAX];
static toggle_t *toggle_map[GPIO_NUM_MAX];
static bool toggle_claimed[GPIO_NUM_MAX];
static bool toggles_initialized = false;
static const char *TAG = "toggle";


static void toggle_debounce_timer_callback(TimerHandle_t timer) {
        toggle_t *toggle = (toggle_t*) pvTimerGetTimerID(timer);
        if (!toggle)
                return;

        toggle->debounce_timer_armed = false;

        bool high = my_gpio_read(toggle->gpio_num) == 1;
        if (high != toggle->last_high) {
                toggle->last_high = high;
                toggle->callback(high, toggle->context);
        }
}


static void IRAM_ATTR toggle_gpio_isr_handler(void *arg) {
        toggle_t *toggle = (toggle_t*) arg;
        if (!toggle || !toggle->debounce_timer)
                return;

        BaseType_t higher_task_woken = pdFALSE;
        BaseType_t result;

        if (!toggle->debounce_timer_armed) {
                toggle->debounce_timer_armed = true;
                result = xTimerStartFromISR(toggle->debounce_timer, &higher_task_woken);
                if (result != pdPASS) {
                        toggle->debounce_timer_armed = false;
                }
        } else {
                result = xTimerResetFromISR(toggle->debounce_timer, &higher_task_woken);
        }

        if (result == pdPASS && higher_task_woken == pdTRUE) {
                portYIELD_FROM_ISR();
        }
}


static toggle_t *toggle_find_by_gpio(const gpio_num_t gpio_num) {
        if (!GPIO_IS_VALID_GPIO(gpio_num))
                return NULL;

        return toggle_map[(size_t) gpio_num];
}


static int toggles_init() {
        if (toggles_initialized)
                return 0;

        toggles_lock = xSemaphoreCreateMutex();
        if (!toggles_lock) {
                ESP_LOGE(TAG, "Failed to create toggle lock");
                return -1;
        }

        esp_err_t err = gpio_install_isr_service(ESP_INTR_FLAG_LOWMED);
        if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
                ESP_LOGE(TAG, "Failed to install GPIO ISR service: %s", esp_err_to_name(err));
                vSemaphoreDelete(toggles_lock);
                toggles_lock = NULL;
                return -1;
        }

        toggles_initialized = true;
        return 0;
}


int toggle_create(const gpio_num_t gpio_num, toggle_callback_fn callback, void* context) {
        if (!toggles_initialized) {
                if (toggles_init() != 0)
                        return -3;
        }

        if (!GPIO_IS_VALID_GPIO(gpio_num)) {
                ESP_LOGE(TAG, "Invalid GPIO number: %d", (int) gpio_num);
                return -2;
        }

        const size_t index = (size_t) gpio_num;

        toggle_t *toggle = &toggle_pool[index];

        xSemaphoreTake(toggles_lock, portMAX_DELAY);
        if (toggle_claimed[index]) {
                xSemaphoreGive(toggles_lock);
                return -1;
        }
        toggle_claimed[index] = true;
        xSemaphoreGive(toggles_lock);

        memset(toggle, 0, sizeof(*toggle));
        toggle->gpio_num = gpio_num;
        toggle->callback = callback;
        toggle->context = context;

        toggle->debounce_timer = xTimerCreateStatic(
                "Toggle debounce",
                pdMS_TO_TICKS(TOGGLE_DEBOUNCE_MS),
                pdFALSE,
                toggle,
                toggle_debounce_timer_callback,
                &toggle_timer_buffers[index]
        );
        if (!toggle->debounce_timer) {
                ESP_LOGE(TAG, "Failed to create debounce timer for GPIO %d", (int) gpio_num);
                goto fail_no_timer;
        }

        bool intr_type_set = false;
        bool isr_registered = false;
        bool intr_enabled = false;

        my_gpio_enable(toggle->gpio_num);
        toggle->last_high = my_gpio_read(toggle->gpio_num) == 1;

        esp_err_t err = gpio_set_intr_type(toggle->gpio_num, GPIO_INTR_ANYEDGE);
        if (err != ESP_OK) {
                ESP_LOGE(TAG, "Failed to set interrupt type for GPIO %d: %s", (int) gpio_num, esp_err_to_name(err));
                goto fail;
        }
        intr_type_set = true;

        err = gpio_isr_handler_add(toggle->gpio_num, toggle_gpio_isr_handler, toggle);
        if (err != ESP_OK) {
                ESP_LOGE(TAG, "Failed to add ISR handler for GPIO %d: %s", (int) gpio_num, esp_err_to_name(err));
                goto fail;
        }
        isr_registered = true;

        err = gpio_intr_enable(toggle->gpio_num);
        if (err != ESP_OK) {
                ESP_LOGE(TAG, "Failed to enable interrupts for GPIO %d: %s", (int) gpio_num, esp_err_to_name(err));
                goto fail;
        }
        intr_enabled = true;

        xSemaphoreTake(toggles_lock, portMAX_DELAY);
        toggle_map[index] = toggle;
        toggle_claimed[index] = true;
        xSemaphoreGive(toggles_lock);

        return 0;

fail:
        if (intr_enabled) {
                gpio_intr_disable(toggle->gpio_num);
        }
        if (isr_registered) {
                esp_err_t remove_err = gpio_isr_handler_remove(toggle->gpio_num);
                if (remove_err != ESP_OK && remove_err != ESP_ERR_INVALID_STATE) {
                        ESP_LOGE(TAG, "Failed to remove ISR handler for GPIO %d: %s", (int) gpio_num, esp_err_to_name(remove_err));
                }
        }
        if (intr_type_set) {
                gpio_set_intr_type(toggle->gpio_num, GPIO_INTR_DISABLE);
        }

        if (toggle->debounce_timer) {
                toggle->debounce_timer_armed = false;
                xTimerStop(toggle->debounce_timer, 0);
                xTimerDelete(toggle->debounce_timer, 0);
                toggle->debounce_timer = NULL;
        }

fail_no_timer:
        xSemaphoreTake(toggles_lock, portMAX_DELAY);
        toggle_map[index] = NULL;
        toggle_claimed[index] = false;
        xSemaphoreGive(toggles_lock);

        memset(toggle, 0, sizeof(*toggle));

        return -4;
}


void toggle_delete(const gpio_num_t gpio_num) {
        if (!toggles_initialized)
                return;

        if (!GPIO_IS_VALID_GPIO(gpio_num)) {
                ESP_LOGE(TAG, "Invalid GPIO number: %d", (int) gpio_num);
                return;
        }

        toggle_t *toggle = NULL;
        const size_t index = (size_t) gpio_num;

        xSemaphoreTake(toggles_lock, portMAX_DELAY);

        toggle = toggle_map[index];
        if (!toggle) {
                // No active toggle to delete; leave the claimed state untouched in case
                // a concurrent creation is still in progress for this GPIO.
                xSemaphoreGive(toggles_lock);
                return;
        }

        toggle_map[index] = NULL;

        esp_err_t err = gpio_intr_disable(gpio_num);
        if (err != ESP_OK) {
                ESP_LOGE(TAG, "Failed to disable interrupts for GPIO %d: %s", (int) gpio_num, esp_err_to_name(err));
        }

        err = gpio_isr_handler_remove(gpio_num);
        if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
                ESP_LOGE(TAG, "Failed to remove ISR handler for GPIO %d: %s", (int) gpio_num, esp_err_to_name(err));
        }
        gpio_set_intr_type(gpio_num, GPIO_INTR_DISABLE);

        if (toggle->debounce_timer) {
                toggle->debounce_timer_armed = false;
                xTimerStop(toggle->debounce_timer, 0);
                xTimerDelete(toggle->debounce_timer, 0);
                toggle->debounce_timer = NULL;
        }

        toggle_claimed[index] = false;

        memset(toggle, 0, sizeof(*toggle));

        xSemaphoreGive(toggles_lock);
}


void toggle_sync_state(const gpio_num_t gpio_num) {
        if (!toggles_initialized)
                return;

        xSemaphoreTake(toggles_lock, portMAX_DELAY);

        toggle_t *toggle = toggle_find_by_gpio(gpio_num);
        if (toggle) {
                if (toggle->debounce_timer && xTimerIsTimerActive(toggle->debounce_timer)) {
                        xTimerStop(toggle->debounce_timer, 0);
                }

                toggle->debounce_timer_armed = false;

                toggle->last_high = my_gpio_read(toggle->gpio_num) == 1;
        }

        xSemaphoreGive(toggles_lock);
}
