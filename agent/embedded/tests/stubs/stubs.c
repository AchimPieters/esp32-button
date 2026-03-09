#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "esp_err.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "port.h"

struct FakeSemaphore {
        int dummy;
};

SemaphoreHandle_t xSemaphoreCreateMutex(void) {
        struct FakeSemaphore *sem = malloc(sizeof(*sem));
        return sem;
}

void vSemaphoreDelete(SemaphoreHandle_t semaphore) {
        free(semaphore);
}

BaseType_t xSemaphoreTake(SemaphoreHandle_t semaphore, TickType_t ticks_to_wait) {
        (void) ticks_to_wait;
        return semaphore ? pdTRUE : pdFALSE;
}

BaseType_t xSemaphoreGive(SemaphoreHandle_t semaphore) {
        return semaphore ? pdTRUE : pdFALSE;
}

TimerHandle_t xTimerCreateStatic(const char * const name,
                                 TickType_t period_in_ticks,
                                 UBaseType_t auto_reload,
                                 void * const timer_id,
                                 TimerCallbackFunction_t callback,
                                 StaticTimer_t *timer_buffer) {
        (void) name;
        (void) period_in_ticks;
        (void) auto_reload;

        if (!timer_buffer)
                return NULL;

        timer_buffer->id = timer_id;
        timer_buffer->callback = callback;
        timer_buffer->active = pdFALSE;

        return timer_buffer;
}

BaseType_t xTimerStartFromISR(TimerHandle_t timer, BaseType_t *higher_priority_task_woken) {
        if (!timer)
                return pdFAIL;

        timer->active = pdTRUE;

        if (higher_priority_task_woken)
                *higher_priority_task_woken = pdFALSE;

        return pdPASS;
}

BaseType_t xTimerResetFromISR(TimerHandle_t timer, BaseType_t *higher_priority_task_woken) {
        if (!timer)
                return pdFAIL;

        timer->active = pdTRUE;

        if (higher_priority_task_woken)
                *higher_priority_task_woken = pdFALSE;

        return pdPASS;
}

BaseType_t xTimerStop(TimerHandle_t timer, TickType_t ticks_to_wait) {
        (void) ticks_to_wait;
        if (!timer)
                return pdFAIL;

        timer->active = pdFALSE;
        return pdPASS;
}

BaseType_t xTimerDelete(TimerHandle_t timer, TickType_t ticks_to_wait) {
        (void) timer;
        (void) ticks_to_wait;
        return pdPASS;
}

BaseType_t xTimerIsTimerActive(TimerHandle_t timer) {
        if (!timer)
                return pdFALSE;

        return timer->active;
}

void *pvTimerGetTimerID(TimerHandle_t timer) {
        if (!timer)
                return NULL;

        return timer->id;
}

static gpio_isr_t s_isr_handlers[GPIO_NUM_MAX];
static bool s_intr_enabled[GPIO_NUM_MAX];
static gpio_int_type_t s_intr_types[GPIO_NUM_MAX];
static uint32_t s_gpio_levels[GPIO_NUM_MAX];

esp_err_t gpio_install_isr_service(int flags) {
        (void) flags;
        return ESP_OK;
}

esp_err_t gpio_set_intr_type(gpio_num_t gpio_num, gpio_int_type_t intr_type) {
        if (!GPIO_IS_VALID_GPIO(gpio_num))
                return ESP_FAIL;

        s_intr_types[gpio_num] = intr_type;
        return ESP_OK;
}

esp_err_t gpio_isr_handler_add(gpio_num_t gpio_num, gpio_isr_t isr_handler, void *args) {
        (void) args;
        if (!GPIO_IS_VALID_GPIO(gpio_num))
                return ESP_FAIL;

        s_isr_handlers[gpio_num] = isr_handler;
        return ESP_OK;
}

esp_err_t gpio_isr_handler_remove(gpio_num_t gpio_num) {
        if (!GPIO_IS_VALID_GPIO(gpio_num))
                return ESP_FAIL;

        s_isr_handlers[gpio_num] = NULL;
        return ESP_OK;
}

esp_err_t gpio_intr_enable(gpio_num_t gpio_num) {
        if (!GPIO_IS_VALID_GPIO(gpio_num))
                return ESP_FAIL;

        s_intr_enabled[gpio_num] = true;
        return ESP_OK;
}

esp_err_t gpio_intr_disable(gpio_num_t gpio_num) {
        if (!GPIO_IS_VALID_GPIO(gpio_num))
                return ESP_FAIL;

        s_intr_enabled[gpio_num] = false;
        return ESP_OK;
}

uint32_t gpio_get_level(gpio_num_t gpio_num) {
        if (!GPIO_IS_VALID_GPIO(gpio_num))
                return 0;

        return s_gpio_levels[gpio_num];
}

void my_gpio_enable(gpio_num_t gpio) {
        (void) gpio;
}

uint8_t my_gpio_read(gpio_num_t gpio) {
        return (uint8_t) gpio_get_level(gpio);
}

const char *esp_err_to_name(esp_err_t err) {
        switch (err) {
        case ESP_OK:
                return "ESP_OK";
        case ESP_ERR_INVALID_STATE:
                return "ESP_ERR_INVALID_STATE";
        case ESP_FAIL:
                return "ESP_FAIL";
        default:
                return "ESP_ERR_UNKNOWN";
        }
}
