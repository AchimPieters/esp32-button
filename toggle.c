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
        TimerHandle_t debounce_timer;

        struct _toggle *next;
} toggle_t;


#define TOGGLE_DEBOUNCE_MS 10


static SemaphoreHandle_t toggles_lock = NULL;
static toggle_t *toggles = NULL;
static bool toggles_initialized = false;
static const char *TAG = "toggle";


static void toggle_debounce_timer_callback(TimerHandle_t timer) {
        toggle_t *toggle = (toggle_t*) pvTimerGetTimerID(timer);
        if (!toggle)
                return;

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
        if (xTimerResetFromISR(toggle->debounce_timer, &higher_task_woken) == pdPASS) {
                if (higher_task_woken == pdTRUE) {
                        portYIELD_FROM_ISR();
                }
        }
}


static toggle_t *toggle_find_by_gpio(const gpio_num_t gpio_num) {
        toggle_t *toggle = toggles;
        while (toggle && toggle->gpio_num != gpio_num)
                toggle = toggle->next;

        return toggle;
}


static int toggles_init() {
        if (toggles_initialized)
                return 0;

        toggles_lock = xSemaphoreCreateBinary();
        if (!toggles_lock) {
                ESP_LOGE(TAG, "Failed to create toggle lock");
                return -1;
        }
        xSemaphoreGive(toggles_lock);

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

        toggle_t *toggle = toggle_find_by_gpio(gpio_num);
        if (toggle)
                return -1;

        toggle = malloc(sizeof(toggle_t));
        if (!toggle) {
                ESP_LOGE(TAG, "Failed to allocate memory for toggle on GPIO %d", (int) gpio_num);
                return -3;
        }

        memset(toggle, 0, sizeof(*toggle));
        toggle->gpio_num = gpio_num;
        toggle->callback = callback;
        toggle->context = context;

        toggle->debounce_timer = xTimerCreate(
                "Toggle debounce", pdMS_TO_TICKS(TOGGLE_DEBOUNCE_MS), pdFALSE, toggle, toggle_debounce_timer_callback
                );
        if (!toggle->debounce_timer) {
                ESP_LOGE(TAG, "Failed to create debounce timer for GPIO %d", (int) gpio_num);
                free(toggle);
                return -4;
        }

        my_gpio_enable(toggle->gpio_num);
        toggle->last_high = my_gpio_read(toggle->gpio_num) == 1;

        esp_err_t err = gpio_set_intr_type(toggle->gpio_num, GPIO_INTR_ANYEDGE);
        if (err != ESP_OK) {
                ESP_LOGE(TAG, "Failed to set interrupt type for GPIO %d: %s", (int) gpio_num, esp_err_to_name(err));
                xTimerDelete(toggle->debounce_timer, portMAX_DELAY);
                free(toggle);
                return -4;
        }

        err = gpio_isr_handler_add(toggle->gpio_num, toggle_gpio_isr_handler, toggle);
        if (err != ESP_OK) {
                ESP_LOGE(TAG, "Failed to add ISR handler for GPIO %d: %s", (int) gpio_num, esp_err_to_name(err));
                gpio_set_intr_type(toggle->gpio_num, GPIO_INTR_DISABLE);
                xTimerDelete(toggle->debounce_timer, portMAX_DELAY);
                free(toggle);
                return -4;
        }

        err = gpio_intr_enable(toggle->gpio_num);
        if (err != ESP_OK) {
                ESP_LOGE(TAG, "Failed to enable interrupts for GPIO %d: %s", (int) gpio_num, esp_err_to_name(err));
                gpio_isr_handler_remove(toggle->gpio_num);
                gpio_set_intr_type(toggle->gpio_num, GPIO_INTR_DISABLE);
                xTimerDelete(toggle->debounce_timer, portMAX_DELAY);
                free(toggle);
                return -4;
        }

        xSemaphoreTake(toggles_lock, portMAX_DELAY);

        toggle->next = toggles;
        toggles = toggle;

        xSemaphoreGive(toggles_lock);

        return 0;
}


void toggle_delete(const gpio_num_t gpio_num) {
        if (!toggles_initialized) {
                if (toggles_init() != 0)
                        return;
        }

        if (!GPIO_IS_VALID_GPIO(gpio_num)) {
                ESP_LOGE(TAG, "Invalid GPIO number: %d", (int) gpio_num);
                return;
        }

        xSemaphoreTake(toggles_lock, portMAX_DELAY);

        if (!toggles) {
                xSemaphoreGive(toggles_lock);
                return;
        }

        toggle_t *toggle = NULL;
        if (toggles->gpio_num == gpio_num) {
                toggle = toggles;
                toggles = toggles->next;
        } else {
                toggle_t *b = toggles;
                while (b->next) {
                        if (b->next->gpio_num == gpio_num) {
                                toggle = b->next;
                                b->next = b->next->next;
                                break;
                        }
                }
        }

        xSemaphoreGive(toggles_lock);

        if (!toggle)
                return;

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
                xTimerStop(toggle->debounce_timer, portMAX_DELAY);
                xTimerDelete(toggle->debounce_timer, portMAX_DELAY);
        }

        free(toggle);
}


void toggle_sync_state(const gpio_num_t gpio_num) {
        if (!toggles_initialized)
                return;

        xSemaphoreTake(toggles_lock, portMAX_DELAY);

        toggle_t *toggle = toggle_find_by_gpio(gpio_num);
        if (toggle) {
                if (toggle->debounce_timer && xTimerIsTimerActive(toggle->debounce_timer)) {
                        xTimerStop(toggle->debounce_timer, portMAX_DELAY);
                }

                toggle->last_high = my_gpio_read(toggle->gpio_num) == 1;
        }

        xSemaphoreGive(toggles_lock);
}
