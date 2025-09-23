#ifndef TOGGLE_H
#define TOGGLE_H

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <driver/gpio.h>

// Callback function signature
typedef void (*toggle_callback_fn)(bool high, void* context);

// Function to create a toggle
// Returns 0 on success, -1 if the GPIO is already tracked, -2 if the GPIO is invalid,
// -3 if the toggle subsystem cannot be initialised, and -4 if timer or interrupt
// configuration fails.
int toggle_create(gpio_num_t gpio_num, toggle_callback_fn callback, void* context);

// Function to delete a toggle
void toggle_delete(gpio_num_t gpio_num);

// Force the toggle helper to resample and synchronise its state without
// generating callbacks.
void toggle_sync_state(gpio_num_t gpio_num);

#endif // TOGGLE_H
