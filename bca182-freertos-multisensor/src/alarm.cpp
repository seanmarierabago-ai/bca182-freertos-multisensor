#include "alarm.h"

#include "alarm_logic.h"
#include "hardware.h"
#include "rtos_objects.h"
#include "serial_log.h"
#include "system_state.h"

void AlarmTask(void *pvParameters)
{
    SensorData sensorData = {};

    for (;;) {
        if (xQueueReceive(alarmSensorQueue, &sensorData, portMAX_DELAY) == pdPASS) {
            if (!sensorData.dhtValid) {
                SystemState_SetAlarmActive(false);
                Buzzer_Set(false);
                Serial_Print("AlarmTask: temperature unavailable\r\n");
            } else {
                const AlarmState alarmState = evaluateTemperature(sensorData.temperature);
                const bool alarmActive = alarmState != AlarmState::NORMAL;
                SystemState_SetAlarmActive(alarmActive);
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