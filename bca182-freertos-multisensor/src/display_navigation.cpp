#include "display_navigation.h"

DisplayMode nextDisplayMode(DisplayMode currentMode)
{
    switch (currentMode) {
    case DisplayMode::TEMPERATURE: return DisplayMode::HUMIDITY;
    case DisplayMode::HUMIDITY: return DisplayMode::LIGHT;
    case DisplayMode::LIGHT: return DisplayMode::MOTION;
    case DisplayMode::MOTION:
    default: return DisplayMode::TEMPERATURE;
    }
}

DisplayMode previousDisplayMode(DisplayMode currentMode)
{
    switch (currentMode) {
    case DisplayMode::TEMPERATURE: return DisplayMode::MOTION;
    case DisplayMode::HUMIDITY: return DisplayMode::TEMPERATURE;
    case DisplayMode::LIGHT: return DisplayMode::HUMIDITY;
    case DisplayMode::MOTION: return DisplayMode::LIGHT;
    default: return DisplayMode::TEMPERATURE;
    }
}