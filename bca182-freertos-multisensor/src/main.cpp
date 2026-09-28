#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "hardware.h"
#include "serial_log.h"
#include "rtos_objects.h"
#include "sensors.h"
#include "display.h"
#include "input.h"
#include "alarm.h"
#include "motion.h"
#include "system_state.h"

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

int main(void) {
    // THE FIX: Turn on the serial port BEFORE initializing the hardware
    Serial_EarlyInit();
    Hardware_Init(); 

    if (!RtosObjects_Create()) {
        Serial_WriteRaw("RTOS object creation failed\r\n");
        while (1) {
        }
    }

    SystemState_Init();

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