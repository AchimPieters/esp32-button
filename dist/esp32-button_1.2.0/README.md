# ESP32 Button Library

ESP32 Button library for handling button input events such as single press, double press, and long press.  
Designed for use with **ESP-IDF**.

---

## Features

This component provides a simple interface to connect a (hardware) button to a GPIO pin and handle events such as:
- Single press  
- Double press  
- Long press  

Configuration can be done via `menuconfig` (GPIO pin, active low/high, which events are enabled, repeat press detection, etc.).

---

## Configuration

To configure the library:

1. Run `idf.py menuconfig`  
2. Navigate to the **Button** / **esp32-button** component configuration  
3. Configure the following options:

| Setting            | Description                                     | Default value |
|--------------------|-------------------------------------------------|---------------|
| GPIO Number        | The GPIO pin where the button is connected      | `0`           |
| Active Low/High    | Defines if the button is triggered on LOW or HIGH | `Active Low`  |
| Long Press Time (ms) | Time in milliseconds to trigger a long press  | `1000`        |
| Max Repeat Presses | Number of repeated presses to recognize (e.g. double, triple) | `3` |

---

## Wiring

Connect the button as follows:

- **Button pin** → **GPIO** (configured in `menuconfig`)  
- **Other side of the button** → GND (or VCC, depending on active_low / active_high setting)  
- Use either an external pull-up/pull-down resistor or the ESP32’s internal pull-up/pull-down.

---

## Example Code

```c
#include <driver/gpio.h>
#include "button.h"

#define BUTTON_GPIO CONFIG_ESP_BUTTON_GPIO

void button_callback(button_event_t event, void *context) {
    switch (event) {
    case button_event_single_press:
        ESP_LOGI("BUTTON", "Single press");
        break;
    case button_event_double_press:
        ESP_LOGI("BUTTON", "Double press");
        break;
    case button_event_long_press:
        ESP_LOGI("BUTTON", "Long press");
        break;
    default:
        ESP_LOGI("BUTTON", "Unknown button event: %d", event);
        break;
    }
}

void app_main(void)
{
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Woverride-init"
    button_config_t config = BUTTON_CONFIG(
        button_active_low,
        .max_repeat_presses = 3,
        .long_press_time = 1000
    );
    #pragma GCC diagnostic pop

    if (button_create(BUTTON_GPIO, config, button_callback, NULL)) {
        ESP_LOGE("BUTTON", "Failed to initialize button");
    }

    // Example main loop
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
```

---

## Build and Run

Build the project with:  
```bash
idf.py build
```

Flash the ESP32:  
```bash
idf.py flash
```

Open the serial monitor:  
```bash
idf.py monitor
```

---

## Example Output

```
I (12345) BUTTON: Single press
I (14345) BUTTON: Double press
I (20345) BUTTON: Long press
```

---

## Notes

- Make sure wiring matches your `menuconfig` settings  
- Adjust `long_press_time` and `max_repeat_presses` to tune responsiveness  
- Use internal pull-up/pull-down if needed, or add external resistors  

---

StudioPieters® | Innovation and Learning Labs | https://www.studiopieters.nl
