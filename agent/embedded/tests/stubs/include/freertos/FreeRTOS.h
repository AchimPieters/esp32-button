#ifndef FREERTOS_FREERTOS_H
#define FREERTOS_FREERTOS_H

#include <stdint.h>

typedef int BaseType_t;
typedef unsigned int UBaseType_t;
typedef uint32_t TickType_t;

typedef int32_t portBASE_TYPE;

typedef uint32_t StackType_t;

typedef uint32_t StaticQueue_t;

typedef uint32_t StaticSemaphore_t;

#define pdFALSE ((BaseType_t)0)
#define pdTRUE ((BaseType_t)1)
#define pdPASS (pdTRUE)
#define pdFAIL (pdFALSE)
#define portMAX_DELAY ((TickType_t)0xffffffffu)
#define pdMS_TO_TICKS(ms) (ms)
#define portYIELD_FROM_ISR() do { } while (0)

#endif // FREERTOS_FREERTOS_H
