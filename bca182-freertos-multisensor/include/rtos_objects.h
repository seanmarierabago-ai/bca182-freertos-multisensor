#pragma once

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"

#include "app_types.h"

extern QueueHandle_t displaySensorQueue;
extern QueueHandle_t alarmSensorQueue;
extern QueueHandle_t displayModeQueue;
extern QueueHandle_t systemStateQueue;
extern QueueSetHandle_t displayQueueSet;
extern SemaphoreHandle_t serialMutex;
extern EventGroupHandle_t systemEvents;

/* MotionTask sets/clears ACTIVE and MOTION; InputTask and DisplayTask consume them.
	AlarmTask sets/clears ALARM; DisplayTask consumes it for transition logging. */
constexpr EventBits_t EVENT_ACTIVE = (1U << 0);
constexpr EventBits_t EVENT_MOTION = (1U << 1);
constexpr EventBits_t EVENT_ALARM = (1U << 2);

bool RtosObjects_Create(void);
