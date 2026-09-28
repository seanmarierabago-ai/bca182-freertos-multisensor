#include "rtos_objects.h"

QueueHandle_t displaySensorQueue = nullptr;
QueueHandle_t alarmSensorQueue = nullptr;
QueueHandle_t displayModeQueue = nullptr;
QueueHandle_t motionStateQueue = nullptr;
QueueSetHandle_t displayQueueSet = nullptr;
SemaphoreHandle_t serialMutex = nullptr;
EventGroupHandle_t systemEvents = nullptr;

bool RtosObjects_Create(void)
{
    displaySensorQueue = xQueueCreate(1U, sizeof(SensorData));
    alarmSensorQueue = xQueueCreate(1U, sizeof(SensorData));
    displayModeQueue = xQueueCreate(4U, sizeof(DisplayMode));
    motionStateQueue = xQueueCreate(1U, sizeof(bool));
    serialMutex = xSemaphoreCreateMutex();
    systemEvents = xEventGroupCreate();
    displayQueueSet = xQueueCreateSet(5U);

    if (displayQueueSet != nullptr && displaySensorQueue != nullptr && displayModeQueue != nullptr) {
        xQueueAddToSet(displaySensorQueue, displayQueueSet);
        xQueueAddToSet(displayModeQueue, displayQueueSet);
    }

    return displaySensorQueue != nullptr &&
           alarmSensorQueue != nullptr &&
           displayModeQueue != nullptr &&
           motionStateQueue != nullptr &&
           serialMutex != nullptr &&
           systemEvents != nullptr &&
           displayQueueSet != nullptr;
}