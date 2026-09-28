#include "input.h"

#include "hardware.h"
#include "rtos_objects.h"
#include "system_state.h"

void InputTask(void *pvParameters)
{
    DisplayMode currentMode = DisplayMode::TEMPERATURE;
    constexpr uint8_t modeCount = 4U;
    TickType_t lastWakeTime = xTaskGetTickCount();

    for (;;) {
        if (SystemState_IsActive()) {
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