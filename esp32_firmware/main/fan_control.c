#include "fan_control.h"

FanCommand fan_control_calculate(float temperature_c)
{
    if (temperature_c < 40.0f) {
        return (FanCommand){.fanMode = FAN_OFF, .dutyCycle = 0};
    }

    if (temperature_c < 70.0f) {
        return (FanCommand){.fanMode = FAN_LOW, .dutyCycle = 40};
    }

    if (temperature_c < 90.0f) {
        return (FanCommand){.fanMode = FAN_MEDIUM, .dutyCycle = 70};
    }

    return (FanCommand){.fanMode = FAN_HIGH, .dutyCycle = 100};
}
