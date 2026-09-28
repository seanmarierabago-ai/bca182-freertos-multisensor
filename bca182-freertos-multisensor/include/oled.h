#pragma once

#include "app_types.h"

bool Oled_Init(void);
bool Oled_SetEnabled(bool enabled);
bool Oled_ShowPage(DisplayMode mode, const SensorData *sensorData);