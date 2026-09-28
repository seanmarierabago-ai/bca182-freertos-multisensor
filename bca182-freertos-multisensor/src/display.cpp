#include "display.h"

#include "oled.h"
#include "rtos_objects.h"
#include "serial_log.h"
#include "system_state.h"

void DisplayTask(void *pvParameters)
{
    SensorData latestSensorData = {};
    DisplayMode currentMode = DisplayMode::TEMPERATURE;
    bool haveSensorData = false;
    bool systemActive = true;
    bool previousAlarmActive = false;

    const bool oledReady = Oled_Init();
    if (!oledReady) {
        Serial_Print("OLED initialization failed\r\n");
    } else {
        Serial_Print("OLED initialized\r\n");
        Oled_ShowPage(currentMode, nullptr);
    }

    for (;;) {
        const QueueSetMemberHandle_t selected = xQueueSelectFromSet(displayQueueSet, portMAX_DELAY);
        if (selected == displaySensorQueue) {
            if (xQueueReceive(displaySensorQueue, &latestSensorData, 0U) == pdPASS) {
                haveSensorData = true;
                latestSensorData.motionDetected = SystemState_IsMotionDetected();
                if (systemActive && oledReady) {
                    Oled_ShowPage(currentMode, &latestSensorData);
                }
            }
        } else if (selected == displayModeQueue) {
            xQueueReceive(displayModeQueue, &currentMode, 0U);
            if (systemActive && oledReady) {
                Oled_ShowPage(currentMode, haveSensorData ? &latestSensorData : nullptr);
            }
        } else if (selected == systemStateQueue) {
            SystemState nextState = SystemState::ACTIVE;
            if (xQueueReceive(systemStateQueue, &nextState, 0U) == pdPASS) {
                systemActive = SystemState_IsActive();
                if (oledReady) {
                    if (systemActive) {
                        Oled_SetEnabled(true);
                        Oled_ShowPage(currentMode, haveSensorData ? &latestSensorData : nullptr);
                    } else {
                        Oled_SetEnabled(false);
                    }
                }
                Serial_Print(systemActive ? "System ACTIVE: motion detected\r\n"
                                         : "System INACTIVE: OLED sleeping\r\n");
            }
        }

        const bool alarmActive = SystemState_IsAlarmActive();
        if (alarmActive != previousAlarmActive) {
            previousAlarmActive = alarmActive;
            Serial_Print(alarmActive ? "EVENT_ALARM set\r\n" : "EVENT_ALARM cleared\r\n");
        }
    }
}