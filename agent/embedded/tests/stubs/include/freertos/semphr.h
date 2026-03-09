#ifndef FREERTOS_SEMPHR_H
#define FREERTOS_SEMPHR_H

#include "FreeRTOS.h"

typedef struct FakeSemaphore* SemaphoreHandle_t;

SemaphoreHandle_t xSemaphoreCreateMutex(void);
void vSemaphoreDelete(SemaphoreHandle_t semaphore);
BaseType_t xSemaphoreTake(SemaphoreHandle_t semaphore, TickType_t ticks_to_wait);
BaseType_t xSemaphoreGive(SemaphoreHandle_t semaphore);

#endif // FREERTOS_SEMPHR_H
