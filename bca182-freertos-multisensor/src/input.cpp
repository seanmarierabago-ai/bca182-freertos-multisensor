#include "input.h"

#include "hardware.h"
#include "rtos_objects.h"
#include "system_state.h"
#include "display_navigation.h"

void InputTask(void *pvParameters)
{
    DisplayMode currentMode = DisplayMode::TEMPERATURE;
    TickType_t lastWakeTime = xTaskGetTickCount();

    for (;;) {
        if (SystemState_IsActive()) {
            const int8_t step = Encoder_ReadStep();
            if (step != 0) {
                currentMode = step > 0
                    ? nextDisplayMode(currentMode)
                    : previousDisplayMode(currentMode);
                xQueueSend(displayModeQueue, &currentMode, 0U);
            }
        }
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(10));
    }
}