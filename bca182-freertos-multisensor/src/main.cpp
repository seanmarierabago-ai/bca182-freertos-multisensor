#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "hardware.h"
#include "serial_log.h"
#include "rtos_objects.h"
#include "oled.h"
#include "alarm_logic.h"

#include <stdio.h>
#include <string.h>

extern "C" {
    BaseType_t xPortConsumeTickYield(void);
    void vPortYieldDirect(void);
    void vApplicationIdleHook(void) {
        if (xPortConsumeTickYield() != pdFALSE) {
            vPortYieldDirect();
        }
    }
    void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {}
    void vApplicationMallocFailedHook(void) {}
    void vAssertCalled(const char *pcFile, int ulLine) { while(1); }
}

void SensorTask(void *pvParameters) {
    TickType_t lastWakeTime = xTaskGetTickCount();

    for (;;) {
        SensorData sensorData = {};
        uint16_t lightRaw = 0U;

        sensorData.dhtValid = DHT22_Read(&sensorData.temperature, &sensorData.humidity);
        sensorData.lightValid = LDR_ReadRaw(&lightRaw);
        sensorData.lightLevel = sensorData.lightValid
            ? static_cast<int>((static_cast<uint32_t>(lightRaw) * 100U + 2047U) / 4095U)
            : -1;
        Motion_Read(&sensorData.motionDetected);

        xQueueOverwrite(displaySensorQueue, &sensorData);
        xQueueOverwrite(alarmSensorQueue, &sensorData);

        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(2000));
    }
}

void DisplayTask(void *pvParameters) {
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
                latestSensorData.motionDetected =
                    (xEventGroupGetBits(systemEvents) & EVENT_MOTION) != 0U;
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
                const bool eventGroupActive =
                    (xEventGroupGetBits(systemEvents) & EVENT_ACTIVE) != 0U;
                if (eventGroupActive != systemActive) {
                    systemActive = eventGroupActive;
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
        }

        const bool alarmActive = (xEventGroupGetBits(systemEvents) & EVENT_ALARM) != 0U;
        if (alarmActive != previousAlarmActive) {
            previousAlarmActive = alarmActive;
            Serial_Print(alarmActive ? "EVENT_ALARM set\r\n" : "EVENT_ALARM cleared\r\n");
        }
    }
}

void InputTask(void *pvParameters) {
    DisplayMode currentMode = DisplayMode::TEMPERATURE;
    constexpr uint8_t modeCount = 4U;
    TickType_t lastWakeTime = xTaskGetTickCount();

    for (;;) {
        if ((xEventGroupGetBits(systemEvents) & EVENT_ACTIVE) != 0U) {
            const int8_t step = Encoder_ReadStep();
            if (step != 0) {
                int8_t modeIndex = static_cast<int8_t>(currentMode);
                modeIndex = static_cast<int8_t>((modeIndex + (step > 0 ? 1 : modeCount - 1)) % modeCount);
                currentMode = static_cast<DisplayMode>(modeIndex);
                xQueueSend(displayModeQueue, &currentMode, 0U);
            }
        }
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(10));
    }
}

void MotionTask(void *pvParameters) {
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
                xEventGroupSetBits(systemEvents, EVENT_MOTION);
            }
            if (currentState == SystemState::INACTIVE) {
                currentState = SystemState::ACTIVE;
                xEventGroupSetBits(systemEvents, EVENT_ACTIVE);
                xQueueOverwrite(systemStateQueue, &currentState);
                Serial_Print("MotionTask: returning ACTIVE\r\n");
            }
        } else {
            if (previousMotion) {
                xEventGroupClearBits(systemEvents, EVENT_MOTION);
            }
            if (currentState == SystemState::ACTIVE &&
                static_cast<TickType_t>(now - lastMotionTime) >= inactivityTimeout) {
                currentState = SystemState::INACTIVE;
                xEventGroupClearBits(systemEvents, EVENT_ACTIVE);
                xQueueOverwrite(systemStateQueue, &currentState);
                Serial_Print("MotionTask: inactivity timeout reached\r\n");
            }
        }

        previousMotion = motionDetected;

        vTaskDelayUntil(&lastWakeTime, motionPollPeriod);
    }
}

void AlarmTask(void *pvParameters) {
    SensorData sensorData = {};

    for (;;) {
        if (xQueueReceive(alarmSensorQueue, &sensorData, portMAX_DELAY) == pdPASS) {
            if (!sensorData.dhtValid) {
                xEventGroupClearBits(systemEvents, EVENT_ALARM);
                Buzzer_Set(false);
                Serial_Print("AlarmTask: temperature unavailable\r\n");
            } else {
                const AlarmState alarmState = evaluateTemperature(sensorData.temperature);
                const bool alarmActive = alarmState != AlarmState::NORMAL;
                if (alarmActive) {
                    xEventGroupSetBits(systemEvents, EVENT_ALARM);
                } else {
                    xEventGroupClearBits(systemEvents, EVENT_ALARM);
                }
                Buzzer_Set(alarmActive);

                switch (alarmState) {
                case AlarmState::LOW_TEMPERATURE:
                    Serial_Print("AlarmTask: LOW TEMPERATURE\r\n");
                    break;
                case AlarmState::HIGH_TEMPERATURE:
                    Serial_Print("AlarmTask: HIGH TEMPERATURE\r\n");
                    break;
                case AlarmState::NORMAL:
                default:
                    Serial_Print("AlarmTask: normal\r\n");
                    break;
                }
            }
        }
    }
}

int main(void) {
    // THE FIX: Turn on the serial port BEFORE initializing the hardware
    Serial_EarlyInit();
    Hardware_Init(); 

    if (!RtosObjects_Create()) {
        Serial_WriteRaw("RTOS object creation failed\r\n");
        while (1) {
        }
    }

    const BaseType_t sensorTaskResult = xTaskCreate(SensorTask, "SensorTask", 256, NULL, 2, NULL);
    const BaseType_t displayTaskResult = xTaskCreate(DisplayTask, "DisplayTask", 256, NULL, 1, NULL);
    const BaseType_t alarmTaskResult = xTaskCreate(AlarmTask, "AlarmTask", 256, NULL, 2, NULL);
    const BaseType_t inputTaskResult = xTaskCreate(InputTask, "InputTask", 192, NULL, 3, NULL);
    const BaseType_t motionTaskResult = xTaskCreate(MotionTask, "MotionTask", 192, NULL, 3, NULL);
    Serial_WriteRaw(sensorTaskResult == pdPASS ? "SensorTask created\r\n" : "SensorTask creation failed\r\n");
    Serial_WriteRaw(displayTaskResult == pdPASS ? "DisplayTask created\r\n" : "DisplayTask creation failed\r\n");
    Serial_WriteRaw(alarmTaskResult == pdPASS ? "AlarmTask created\r\n" : "AlarmTask creation failed\r\n");
    Serial_WriteRaw(inputTaskResult == pdPASS ? "InputTask created\r\n" : "InputTask creation failed\r\n");
    Serial_WriteRaw(motionTaskResult == pdPASS ? "MotionTask created\r\n" : "MotionTask creation failed\r\n");
    vTaskStartScheduler();

    while (1) {}
}
// Part XI: Mutex to protect Serial Monitor