#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "hardware.h"
#include "serial_log.h"
#include "rtos_objects.h"

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

void TaskA(void *pvParameters) {
    while (1) {
        Serial_WriteRaw("Task A is running\r\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void TaskB(void *pvParameters) {
    vTaskDelay(pdMS_TO_TICKS(500));
    while (1) {
        Serial_WriteRaw("Task B is running\r\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

int main(void) {
    // THE FIX: Turn on the serial port BEFORE initializing the hardware
    Serial_EarlyInit();
    Hardware_Init(); 

    const BaseType_t taskAResult = xTaskCreate(TaskA, "TaskA", 128, NULL, 1, NULL);
    const BaseType_t taskBResult = xTaskCreate(TaskB, "TaskB", 128, NULL, 1, NULL);

    Serial_WriteRaw(taskAResult == pdPASS ? "Task A created\r\n" : "Task A creation failed\r\n");
    Serial_WriteRaw(taskBResult == pdPASS ? "Task B created\r\n" : "Task B creation failed\r\n");
    vTaskStartScheduler();

    while (1) {}
}