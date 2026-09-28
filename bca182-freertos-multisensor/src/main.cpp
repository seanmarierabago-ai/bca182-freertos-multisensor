#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "hardware.h"
#include "serial_log.h"
#include "rtos_objects.h"

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
    SensorData sensorData = {};
    char output[128];

    for (;;) {
        if (xQueueReceive(displaySensorQueue, &sensorData, portMAX_DELAY) == pdPASS) {
            if (sensorData.dhtValid) {
                const int32_t temperatureTenths = static_cast<int32_t>(sensorData.temperature * 10.0f +
                    (sensorData.temperature >= 0.0f ? 0.5f : -0.5f));
                const uint16_t humidityTenths = static_cast<uint16_t>(sensorData.humidity * 10.0f + 0.5f);
                const int32_t temperatureFraction = temperatureTenths % 10;
                snprintf(output, sizeof(output),
                         "DisplayTask: %ld.%ld C, %u.%u%% RH, ",
                         static_cast<long>(temperatureTenths / 10),
                         static_cast<long>(temperatureFraction < 0 ? -temperatureFraction : temperatureFraction),
                         static_cast<unsigned>(humidityTenths / 10U),
                         static_cast<unsigned>(humidityTenths % 10U));
            } else {
                snprintf(output, sizeof(output), "DisplayTask: DHT22 unavailable, ");
            }

            const size_t used = strlen(output);
            if (sensorData.lightValid) {
                snprintf(output + used, sizeof(output) - used,
                         "LDR %d%% (scaled, not lux), motion %s\r\n",
                         sensorData.lightLevel,
                         sensorData.motionDetected ? "YES" : "NO");
            } else {
                snprintf(output + used, sizeof(output) - used,
                         "LDR unavailable, motion %s\r\n",
                         sensorData.motionDetected ? "YES" : "NO");
            }
            Serial_Print(output);
        }
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
    Serial_WriteRaw(sensorTaskResult == pdPASS ? "SensorTask created\r\n" : "SensorTask creation failed\r\n");
    Serial_WriteRaw(displayTaskResult == pdPASS ? "DisplayTask created\r\n" : "DisplayTask creation failed\r\n");
    Serial_WriteRaw(alarmTaskResult == pdPASS ? "AlarmTask created\r\n" : "AlarmTask creation failed\r\n");
    vTaskStartScheduler();

    while (1) {}
}