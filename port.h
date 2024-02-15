#ifndef PORT_H
#define PORT_H

#pragma once

#include <stdint.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/timers.h>

void my_gpio_enable(uint8_t gpio);
void my_gpio_pullup(uint8_t gpio);
void my_gpio_pulldown(uint8_t gpio);
uint8_t my_gpio_read(uint8_t gpio);
#endif // PORT_H
