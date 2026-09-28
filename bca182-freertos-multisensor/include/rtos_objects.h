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

bool RtosObjects_Create(void);
