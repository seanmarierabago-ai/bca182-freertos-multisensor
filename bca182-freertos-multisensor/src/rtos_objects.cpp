#include "rtos_objects.h"
#include "system_state.h"

QueueHandle_t displaySensorQueue = nullptr;
QueueHandle_t alarmSensorQueue = nullptr;
QueueHandle_t displayModeQueue = nullptr;
QueueHandle_t systemStateQueue = nullptr;
QueueSetHandle_t displayQueueSet = nullptr;
SemaphoreHandle_t serialMutex = nullptr;
EventGroupHandle_t systemEvents = nullptr;

bool RtosObjects_Create(void)
{
    displaySensorQueue = xQueueCreate(1U, sizeof(SensorData));
    alarmSensorQueue = xQueueCreate(1U, sizeof(SensorData));
    displayModeQueue = xQueueCreate(4U, sizeof(DisplayMode));
    systemStateQueue = xQueueCreate(1U, sizeof(SystemState));
    serialMutex = xSemaphoreCreateMutex();
    systemEvents = xEventGroupCreate();
    displayQueueSet = xQueueCreateSet(6U);

    const bool displayQueuesAdded = displayQueueSet != nullptr &&
        displaySensorQueue != nullptr && displayModeQueue != nullptr &&
        systemStateQueue != nullptr &&
        xQueueAddToSet(displaySensorQueue, displayQueueSet) == pdPASS &&
        xQueueAddToSet(displayModeQueue, displayQueueSet) == pdPASS &&
        xQueueAddToSet(systemStateQueue, displayQueueSet) == pdPASS;

    return displaySensorQueue != nullptr &&
           alarmSensorQueue != nullptr &&
           displayModeQueue != nullptr &&
           systemStateQueue != nullptr &&
           serialMutex != nullptr &&
           systemEvents != nullptr &&
           displayQueueSet != nullptr &&
           displayQueuesAdded;
}