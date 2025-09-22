#ifndef BUTTON_H
#define BUTTON_H

#pragma once

#include <driver/gpio.h>
#include <stdint.h>

typedef enum {
        button_active_low = 0,
        button_active_high = 1,
} button_active_level_t;

typedef struct {
        button_active_level_t active_level;

        // times in milliseconds
        uint16_t long_press_time;
        uint16_t repeat_press_timeout;
        uint16_t max_repeat_presses;
} button_config_t;

typedef enum {
        button_event_single_press,
        button_event_double_press,
        button_event_tripple_press,
        button_event_long_press,
} button_event_t;

typedef void (*button_callback_fn)(button_event_t event, void* context);

#define BUTTON_CONFIG(level, ...) \
        (button_config_t) { \
                .active_level = level, \
                .repeat_press_timeout = 300, \
                .max_repeat_presses = 1, \
                __VA_ARGS__ \
        }

// Returns 0 on success.
// -1 if the GPIO is already registered.
// -2 if the long-press timer cannot be created.
// -3 if the repeat-press timer cannot be created.
// -4 if the GPIO toggle helper cannot be initialised.
// -5 if the GPIO number is invalid.
int button_create(gpio_num_t gpio_num,
                  button_config_t config,
                  button_callback_fn callback,
                  void* context);

void button_destroy(gpio_num_t gpio_num);

#endif // BUTTON_H
