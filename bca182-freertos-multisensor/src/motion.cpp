#include "motion.h"

#include "hardware.h"
#include "rtos_objects.h"
#include "serial_log.h"
#include "system_state.h"

void MotionTask(void *pvParameters)
{
    SystemState currentState = SystemState::ACTIVE;
    TickType_t lastMotionTime = xTaskGetTickCount();
    TickType_t lastWakeTime = lastMotionTime;
    constexpr TickType_t motionPollPeriod = pdMS_TO_TICKS(100);
    const TickType_t inactivityTimeout = pdMS_TO_TICKS(INACTIVITY_TIMEOUT_MS);
    bool previousMotion = false;
    bool firstSample = true;

    Serial_Print("MotionTask monitoring PIR on PA3\r\n");

    for (;;) {
        bool motionDetected = false;
        Motion_Read(&motionDetected);
        const TickType_t now = xTaskGetTickCount();

        if (firstSample || motionDetected != previousMotion) {
            Serial_Print(motionDetected ? "PIR PA3: HIGH (motion)\r\n"
                                        : "PIR PA3: LOW (clear)\r\n");
            firstSample = false;
        }

        if (motionDetected) {
            lastMotionTime = now;
            if (!previousMotion || currentState == SystemState::INACTIVE) {
                SystemState_SetMotionDetected(true);
            }
            if (currentState == SystemState::INACTIVE) {
                currentState = SystemState::ACTIVE;
                SystemState_SetActive(true);
                xQueueOverwrite(systemStateQueue, &currentState);
                Serial_Print("MotionTask: returning ACTIVE\r\n");
            }
        } else {
            if (previousMotion) {
                SystemState_SetMotionDetected(false);
            }
            if (currentState == SystemState::ACTIVE &&
                static_cast<TickType_t>(now - lastMotionTime) >= inactivityTimeout) {
                currentState = SystemState::INACTIVE;
                SystemState_SetActive(false);
                xQueueOverwrite(systemStateQueue, &currentState);
                Serial_Print("MotionTask: inactivity timeout reached\r\n");
            }
        }

        previousMotion = motionDetected;
        vTaskDelayUntil(&lastWakeTime, motionPollPeriod);
    }
}