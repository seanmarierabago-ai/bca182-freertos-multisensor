#pragma once

#include "app_types.h"

SystemState evaluateSystemState(SystemState currentState,
                                bool motionDetected,
                                bool inactivityTimedOut);