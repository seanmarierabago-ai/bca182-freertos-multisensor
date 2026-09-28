#include "system_state_logic.h"

SystemState evaluateSystemState(SystemState currentState,
                                bool motionDetected,
                                bool inactivityTimedOut)
{
    if (motionDetected) {
        return SystemState::ACTIVE;
    }
    if (currentState == SystemState::ACTIVE && inactivityTimedOut) {
        return SystemState::INACTIVE;
    }
    return currentState;
}