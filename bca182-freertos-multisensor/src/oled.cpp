#include "oled.h"

#include "hardware.h"

#include <stdio.h>
#include <string.h>

namespace {

constexpr uint16_t OLED_ADDRESS = 0x3CU << 1U;
constexpr uint16_t OLED_TIMEOUT_MS = 100U;

const uint8_t font[36][5] = {
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, {0x7F, 0x49, 0x49, 0x49, 0x36},
    {0x3E, 0x41, 0x41, 0x41, 0x22}, {0x7F, 0x41, 0x41, 0x22, 0x1C},
    {0x7F, 0x49, 0x49, 0x49, 0x41}, {0x7F, 0x09, 0x09, 0x09, 0x01},
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, {0x7F, 0x08, 0x08, 0x08, 0x7F},
    {0x00, 0x41, 0x7F, 0x41, 0x00}, {0x20, 0x40, 0x41, 0x3F, 0x01},
    {0x7F, 0x08, 0x14, 0x22, 0x41}, {0x7F, 0x40, 0x40, 0x40, 0x40},
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, {0x7F, 0x04, 0x08, 0x10, 0x7F},
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, {0x7F, 0x09, 0x09, 0x09, 0x06},
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, {0x7F, 0x09, 0x19, 0x29, 0x46},
    {0x46, 0x49, 0x49, 0x49, 0x31}, {0x01, 0x01, 0x7F, 0x01, 0x01},
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, {0x1F, 0x20, 0x40, 0x20, 0x1F},
    {0x3F, 0x40, 0x38, 0x40, 0x3F}, {0x63, 0x14, 0x08, 0x14, 0x63},
    {0x07, 0x08, 0x70, 0x08, 0x07}, {0x61, 0x51, 0x49, 0x45, 0x43},
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, {0x00, 0x42, 0x7F, 0x40, 0x00},
    {0x42, 0x61, 0x51, 0x49, 0x46}, {0x21, 0x41, 0x45, 0x4B, 0x31},
    {0x18, 0x14, 0x12, 0x7F, 0x10}, {0x27, 0x45, 0x45, 0x45, 0x39},
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, {0x01, 0x71, 0x09, 0x05, 0x03},
    {0x36, 0x49, 0x49, 0x49, 0x36}, {0x06, 0x49, 0x49, 0x29, 0x1E}
};

bool WriteCommands(const uint8_t *commands, uint8_t length)
{
    uint8_t buffer[32];
    if (length > sizeof(buffer) - 1U) {
        return false;
    }

    buffer[0] = 0x00U;
    memcpy(&buffer[1], commands, length);
    return HAL_I2C_Master_Transmit(&hi2c1, OLED_ADDRESS, buffer,
                                   static_cast<uint16_t>(length + 1U),
                                   OLED_TIMEOUT_MS) == HAL_OK;
}

uint8_t GlyphColumn(char character, uint8_t column)
{
    if (column >= 5U) {
        return 0U;
    }
    if (character >= 'a' && character <= 'z') {
        character = static_cast<char>(character - 'a' + 'A');
    }
    if (character >= 'A' && character <= 'Z') {
        return font[character - 'A'][column];
    }
    if (character >= '0' && character <= '9') {
        return font[26 + character - '0'][column];
    }

    switch (character) {
    case '.': {
        static const uint8_t glyph[5] = {0x00, 0x60, 0x60, 0x00, 0x00};
        return glyph[column];
    }
    case '%': {
        static const uint8_t glyph[5] = {0x63, 0x13, 0x08, 0x64, 0x63};
        return glyph[column];
    }
    case '-': {
        static const uint8_t glyph[5] = {0x08, 0x08, 0x08, 0x08, 0x08};
        return glyph[column];
    }
    default:
        return 0U;
    }
}

bool WriteLine(uint8_t page, const char *text)
{
    const uint8_t position[] = {
        static_cast<uint8_t>(0xB0U | page), 0x00U, 0x10U
    };
    if (!WriteCommands(position, sizeof(position))) {
        return false;
    }

    uint8_t buffer[129] = {};
    buffer[0] = 0x40U;
    size_t outputColumn = 1U;
    for (size_t characterIndex = 0U;
         text[characterIndex] != '\0' && characterIndex < 21U;
         ++characterIndex) {
        for (uint8_t column = 0U; column < 5U; ++column) {
            buffer[outputColumn++] = GlyphColumn(text[characterIndex], column);
        }
        buffer[outputColumn++] = 0U;
    }

    return HAL_I2C_Master_Transmit(&hi2c1, OLED_ADDRESS, buffer,
                                   sizeof(buffer), OLED_TIMEOUT_MS) == HAL_OK;
}

} // namespace

bool Oled_Init(void)
{
    const uint8_t initialization[] = {
        0xAEU, 0xD5U, 0x80U, 0xA8U, 0x3FU, 0xD3U, 0x00U, 0x40U,
        0x8DU, 0x14U, 0x20U, 0x02U, 0xA1U, 0xC8U, 0xDAU, 0x12U,
        0x81U, 0xCFU, 0xD9U, 0xF1U, 0xDBU, 0x40U, 0xA4U, 0xA6U,
        0xAFU
    };
    if (HAL_I2C_IsDeviceReady(&hi2c1, OLED_ADDRESS, 3U, OLED_TIMEOUT_MS) != HAL_OK ||
        !WriteCommands(initialization, sizeof(initialization))) {
        return false;
    }

    const uint8_t clearPosition[] = {0x00U, 0x10U};
    uint8_t blankPage[129] = {};
    blankPage[0] = 0x40U;
    for (uint8_t page = 0U; page < 8U; ++page) {
        const uint8_t position[] = {
            static_cast<uint8_t>(0xB0U | page), clearPosition[0], clearPosition[1]
        };
        if (!WriteCommands(position, sizeof(position)) ||
            HAL_I2C_Master_Transmit(&hi2c1, OLED_ADDRESS, blankPage,
                                    sizeof(blankPage), OLED_TIMEOUT_MS) != HAL_OK) {
            return false;
        }
    }
    return true;
}

bool Oled_SetEnabled(bool enabled)
{
    const uint8_t command = enabled ? 0xAFU : 0xAEU;
    return WriteCommands(&command, 1U);
}

bool Oled_ShowPage(DisplayMode mode, const SensorData *sensorData)
{
    char value[22] = "WAITING FOR DATA";
    const char *title = "TEMPERATURE";

    if (sensorData != nullptr) {
        switch (mode) {
        case DisplayMode::TEMPERATURE:
            title = "TEMPERATURE";
            if (sensorData->dhtValid) {
                const int32_t tenths = static_cast<int32_t>(sensorData->temperature * 10.0f +
                    (sensorData->temperature >= 0.0f ? 0.5f : -0.5f));
                const int32_t fraction = tenths % 10;
                snprintf(value, sizeof(value), "%ld.%ld C",
                         static_cast<long>(tenths / 10),
                         static_cast<long>(fraction < 0 ? -fraction : fraction));
            } else {
                snprintf(value, sizeof(value), "SENSOR ERROR");
            }
            break;
        case DisplayMode::HUMIDITY:
            title = "HUMIDITY";
            if (sensorData->dhtValid) {
                const uint16_t tenths = static_cast<uint16_t>(sensorData->humidity * 10.0f + 0.5f);
                snprintf(value, sizeof(value), "%u.%u %%",
                         static_cast<unsigned>(tenths / 10U),
                         static_cast<unsigned>(tenths % 10U));
            } else {
                snprintf(value, sizeof(value), "SENSOR ERROR");
            }
            break;
        case DisplayMode::LIGHT:
            title = "LIGHT";
            if (sensorData->lightValid) {
                snprintf(value, sizeof(value), "%d%% ADC SCALE", sensorData->lightLevel);
            } else {
                snprintf(value, sizeof(value), "SENSOR ERROR");
            }
            break;
        case DisplayMode::MOTION:
            title = "MOTION";
            snprintf(value, sizeof(value), "%s", sensorData->motionDetected ? "YES" : "NO");
            break;
        }
    }

    return WriteLine(0U, "ROOM MONITOR") &&
           WriteLine(2U, title) &&
           WriteLine(4U, value) &&
           WriteLine(6U, "TURN ENCODER");
}