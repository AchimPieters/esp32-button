#ifndef PORT_H
#define PORT_H

#pragma once

#include <stdint.h>

#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/timers.h>

void my_gpio_enable(gpio_num_t gpio);
void my_gpio_pullup(gpio_num_t gpio);
void my_gpio_pulldown(gpio_num_t gpio);
uint8_t my_gpio_read(gpio_num_t gpio);
#endif // PORT_H
