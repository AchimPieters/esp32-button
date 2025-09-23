#ifndef DRIVER_GPIO_H
#define DRIVER_GPIO_H

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"

typedef int gpio_num_t;
typedef void (*gpio_isr_t)(void*);

typedef enum {
        GPIO_INTR_DISABLE = 0,
        GPIO_INTR_ANYEDGE = 3,
} gpio_int_type_t;

#define GPIO_NUM_MAX 48
#define GPIO_IS_VALID_GPIO(gpio) ((gpio) >= 0 && (gpio) < GPIO_NUM_MAX)

esp_err_t gpio_install_isr_service(int flags);
esp_err_t gpio_set_intr_type(gpio_num_t gpio_num, gpio_int_type_t intr_type);
esp_err_t gpio_isr_handler_add(gpio_num_t gpio_num, gpio_isr_t isr_handler, void *args);
esp_err_t gpio_isr_handler_remove(gpio_num_t gpio_num);
esp_err_t gpio_intr_enable(gpio_num_t gpio_num);
esp_err_t gpio_intr_disable(gpio_num_t gpio_num);
uint32_t gpio_get_level(gpio_num_t gpio_num);

#endif // DRIVER_GPIO_H
