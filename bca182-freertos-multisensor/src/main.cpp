#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "hardware.h"
#include "serial_log.h"
#include "rtos_objects.h"
#include "oled.h"

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

    if (!Oled_Init()) {
        Serial_Print("OLED initialization failed\r\n");
    } else {
        Serial_Print("OLED initialized\r\n");
    }
    Oled_ShowPage(currentMode, nullptr);

    for (;;) {
        const QueueSetMemberHandle_t selected = xQueueSelectFromSet(displayQueueSet, portMAX_DELAY);
        if (selected == displaySensorQueue) {
            if (xQueueReceive(displaySensorQueue, &latestSensorData, 0U) == pdPASS) {
                haveSensorData = true;
            }
        } else if (selected == displayModeQueue) {
            xQueueReceive(displayModeQueue, &currentMode, 0U);
            Serial_Print("Display page changed\r\n");
        }

        if (haveSensorData) {
            Oled_ShowPage(currentMode, &latestSensorData);
        } else {
            Oled_ShowPage(currentMode, nullptr);
        }
    }
}

void InputTask(void *pvParameters) {
    DisplayMode currentMode = DisplayMode::TEMPERATURE;
    constexpr uint8_t modeCount = 4U;
    TickType_t lastWakeTime = xTaskGetTickCount();

    for (;;) {
        const int8_t step = Encoder_ReadStep();
        if (step != 0) {
            int8_t modeIndex = static_cast<int8_t>(currentMode);
            modeIndex = static_cast<int8_t>((modeIndex + (step > 0 ? 1 : modeCount - 1)) % modeCount);
            currentMode = static_cast<DisplayMode>(modeIndex);
            xQueueSend(displayModeQueue, &currentMode, 0U);
        }
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(10));
    }
}

void AlarmTask(void *pvParameters) {
    SensorData sensorData = {};

    for (;;) {
        if (xQueueReceive(alarmSensorQueue, &sensorData, portMAX_DELAY) == pdPASS) {
            if (!sensorData.dhtValid) {
                Serial_Print("AlarmTask: temperature unavailable\r\n");
            } else if (sensorData.temperature < LOW_TEMPERATURE_LIMIT) {
                Serial_Print("AlarmTask: LOW TEMPERATURE\r\n");
            } else if (sensorData.temperature > HIGH_TEMPERATURE_LIMIT) {
                Serial_Print("AlarmTask: HIGH TEMPERATURE\r\n");
            } else {
                Serial_Print("AlarmTask: normal\r\n");
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
    const BaseType_t alarmTaskResult = xTaskCreate(AlarmTask, "AlarmTask", 256, NULL, 1, NULL);
    const BaseType_t inputTaskResult = xTaskCreate(InputTask, "InputTask", 192, NULL, 1, NULL);
    Serial_WriteRaw(sensorTaskResult == pdPASS ? "SensorTask created\r\n" : "SensorTask creation failed\r\n");
    Serial_WriteRaw(displayTaskResult == pdPASS ? "DisplayTask created\r\n" : "DisplayTask creation failed\r\n");
    Serial_WriteRaw(alarmTaskResult == pdPASS ? "AlarmTask created\r\n" : "AlarmTask creation failed\r\n");
    Serial_WriteRaw(inputTaskResult == pdPASS ? "InputTask created\r\n" : "InputTask creation failed\r\n");
    vTaskStartScheduler();

    while (1) {}
}