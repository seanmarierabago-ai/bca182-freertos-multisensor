#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "hardware.h"
#include "serial_log.h"
#include "rtos_objects.h"

#include <stdio.h>

SemaphoreHandle_t serialMutex = NULL;

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
    char output[128];

    for (;;) {
        float temperature = 0.0f;
        float humidity = 0.0f;
        uint16_t lightRaw = 0U;
        const bool dhtValid = DHT22_Read(&temperature, &humidity);
        const bool lightValid = LDR_ReadRaw(&lightRaw);

        if (dhtValid) {
            const int32_t temperatureTenths = static_cast<int32_t>(temperature * 10.0f +
                (temperature >= 0.0f ? 0.5f : -0.5f));
            const uint16_t humidityTenths = static_cast<uint16_t>(humidity * 10.0f + 0.5f);
            const int32_t temperatureFraction = temperatureTenths % 10;
            const int32_t humidityWhole = humidityTenths / 10U;
            const uint16_t humidityFraction = humidityTenths % 10U;
            snprintf(output, sizeof(output),
                     "Temperature: %ld.%ld C, Humidity: %ld.%u %%\r\n",
                     static_cast<long>(temperatureTenths / 10),
                     static_cast<long>(temperatureFraction < 0 ? -temperatureFraction : temperatureFraction),
                     static_cast<long>(humidityWhole),
                     static_cast<unsigned>(humidityFraction));
            Serial_WriteRaw(output);
        } else {
            Serial_WriteRaw("DHT22 read failed (no response or checksum error)\r\n");
        }

        if (lightValid) {
            const uint32_t lightPercent = (static_cast<uint32_t>(lightRaw) * 100U + 2047U) / 4095U;
            snprintf(output, sizeof(output), "LDR ADC: %u/4095, scaled reading: %lu%% (not lux)\r\n",
                     static_cast<unsigned>(lightRaw), static_cast<unsigned long>(lightPercent));
            Serial_WriteRaw(output);
        } else {
            Serial_WriteRaw("LDR ADC read failed\r\n");
        }

        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(2000));
    }
}

int main(void) {
    // THE FIX: Turn on the serial port BEFORE initializing the hardware
    Serial_EarlyInit();
    Hardware_Init(); 

    const BaseType_t sensorTaskResult = xTaskCreate(SensorTask, "SensorTask", 256, NULL, 1, NULL);
    Serial_WriteRaw(sensorTaskResult == pdPASS ? "SensorTask created\r\n" : "SensorTask creation failed\r\n");
    vTaskStartScheduler();

    while (1) {}
}