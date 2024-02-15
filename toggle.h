#ifndef TOGGLE_H
#define TOGGLE_H

#pragma once

#include <stdbool.h>
#include <stdint.h>

// Callback function signature
typedef void (*toggle_callback_fn)(bool high, void* context);

// Function to create a toggle
// Returns 0 on success, non-zero on failure
int toggle_create(uint8_t gpio_num, toggle_callback_fn callback, void* context);

// Function to delete a toggle
void toggle_delete(uint8_t gpio_num);

#endif // TOGGLE_H
