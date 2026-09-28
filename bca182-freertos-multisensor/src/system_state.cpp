#include "system_state.h"

#include "rtos_objects.h"

void SystemState_Init(void)
{
    xEventGroupClearBits(systemEvents, EVENT_MOTION | EVENT_ALARM);
    xEventGroupSetBits(systemEvents, EVENT_ACTIVE);
}

void SystemState_SetActive(bool active)
{
    if (active) {
        xEventGroupSetBits(systemEvents, EVENT_ACTIVE);
    } else {
        xEventGroupClearBits(systemEvents, EVENT_ACTIVE);
    }
}

void SystemState_SetMotionDetected(bool detected)
{
    if (detected) {
        xEventGroupSetBits(systemEvents, EVENT_MOTION);
    } else {
        xEventGroupClearBits(systemEvents, EVENT_MOTION);
    }
}

void SystemState_SetAlarmActive(bool active)
{
    if (active) {
        xEventGroupSetBits(systemEvents, EVENT_ALARM);
    } else {
        xEventGroupClearBits(systemEvents, EVENT_ALARM);
    }
}

bool SystemState_IsActive(void)
{
    return (xEventGroupGetBits(systemEvents) & EVENT_ACTIVE) != 0U;
}

bool SystemState_IsMotionDetected(void)
{
    return (xEventGroupGetBits(systemEvents) & EVENT_MOTION) != 0U;
}

bool SystemState_IsAlarmActive(void)
{
    return (xEventGroupGetBits(systemEvents) & EVENT_ALARM) != 0U;
}