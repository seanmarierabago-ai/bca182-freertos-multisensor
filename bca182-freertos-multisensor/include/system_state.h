#pragma once

#include "FreeRTOS.h"
#include "event_groups.h"
#include "system_state_logic.h"

constexpr EventBits_t EVENT_ACTIVE = (1U << 0);
constexpr EventBits_t EVENT_MOTION = (1U << 1);
constexpr EventBits_t EVENT_ALARM = (1U << 2);

void SystemState_Init(void);
void SystemState_SetActive(bool active);
void SystemState_SetMotionDetected(bool detected);
void SystemState_SetAlarmActive(bool active);
bool SystemState_IsActive(void);
bool SystemState_IsMotionDetected(void);
bool SystemState_IsAlarmActive(void);