#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "hardware.h"
#include "serial_log.h"
#include "rtos_objects.h"

SemaphoreHandle_t serialMutex = NULL;

extern "C" {
    void vApplicationIdleHook(void) {}
    void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {}
    void vApplicationMallocFailedHook(void) {}
    void vAssertCalled(const char *pcFile, int ulLine) { while(1); }
}

void BlinkTask(void *pvParameters) {
    Serial_Print("\r\nSystem running... FreeRTOS is active!\r\n");
    while (1) {
        Serial_Print("Task A is running...\r\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

int main(void) {
    // THE FIX: Turn on the serial port BEFORE initializing the hardware
    Serial_EarlyInit();
    Hardware_Init(); 

    serialMutex = xSemaphoreCreateMutex();

    xTaskCreate(BlinkTask, "Blink", 128, NULL, 1, NULL);
    vTaskStartScheduler();

    while (1) {}
}