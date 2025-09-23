#ifndef FREERTOS_TIMERS_H
#define FREERTOS_TIMERS_H

#include "FreeRTOS.h"

typedef struct StaticTimer StaticTimer_t;
typedef StaticTimer_t* TimerHandle_t;
typedef void (*TimerCallbackFunction_t)(TimerHandle_t timer);

struct StaticTimer {
        void *id;
        TimerCallbackFunction_t callback;
        BaseType_t active;
};

TimerHandle_t xTimerCreateStatic(const char * const name,
                                 TickType_t period_in_ticks,
                                 UBaseType_t auto_reload,
                                 void * const timer_id,
                                 TimerCallbackFunction_t callback,
                                 StaticTimer_t *timer_buffer);
BaseType_t xTimerStartFromISR(TimerHandle_t timer, BaseType_t *higher_priority_task_woken);
BaseType_t xTimerResetFromISR(TimerHandle_t timer, BaseType_t *higher_priority_task_woken);
BaseType_t xTimerStop(TimerHandle_t timer, TickType_t ticks_to_wait);
BaseType_t xTimerDelete(TimerHandle_t timer, TickType_t ticks_to_wait);
BaseType_t xTimerIsTimerActive(TimerHandle_t timer);
void *pvTimerGetTimerID(TimerHandle_t timer);

#endif // FREERTOS_TIMERS_H
