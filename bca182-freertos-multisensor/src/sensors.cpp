#include "sensors.h"

#include "hardware.h"
#include "rtos_objects.h"

void SensorTask(void *pvParameters)
{
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