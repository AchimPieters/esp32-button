#ifndef ESP_LOG_H
#define ESP_LOG_H

#include <stdio.h>

#define ESP_LOGE(TAG, fmt, ...) fprintf(stderr, "E (%s) " fmt "\n", TAG, ##__VA_ARGS__)
#define ESP_LOGI(TAG, fmt, ...) fprintf(stderr, "I (%s) " fmt "\n", TAG, ##__VA_ARGS__)

#endif // ESP_LOG_H
