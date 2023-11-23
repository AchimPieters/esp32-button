#include "port.h"
#include <driver/gpio.h>

// Function to configure GPIO as input
void my_gpio_enable(uint8_t gpio) {
        gpio_set_direction(gpio, GPIO_MODE_INPUT);
}

// Function to set GPIO pullup
void my_gpio_pullup(uint8_t gpio) {
        gpio_set_pull_mode(gpio, GPIO_PULLUP_ONLY);
}

// Function to set GPIO pulldown
void my_gpio_pulldown(uint8_t gpio) {
        gpio_set_pull_mode(gpio, GPIO_PULLDOWN_ONLY);
}

// Function to read GPIO level
uint8_t my_gpio_read(uint8_t gpio) {
        return gpio_get_level(gpio);
}
