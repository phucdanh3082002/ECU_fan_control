#include "fan_control.h"

#define FAN_ON_TEMP_C    20.0f
#define FAN_FULL_TEMP_C  70.0f
#define FAN_DUTY_MIN     20
#define FAN_DUTY_MAX     100

FanCommand fan_control_calculate(float temperature_c)
{
    if (temperature_c < FAN_ON_TEMP_C) {
        return (FanCommand){.fanMode = FAN_OFF, .dutyCycle = 0};
    }

    if (temperature_c >= FAN_FULL_TEMP_C) {
        return (FanCommand){.fanMode = FAN_HIGH, .dutyCycle = FAN_DUTY_MAX};
    }

    float ratio = (temperature_c - FAN_ON_TEMP_C) / (FAN_FULL_TEMP_C - FAN_ON_TEMP_C);
    int duty = FAN_DUTY_MIN + (int)(ratio * (FAN_DUTY_MAX - FAN_DUTY_MIN));

    FanMode mode = FAN_LOW;
    if (duty >= 70) {
        mode = FAN_HIGH;
    } else if (duty >= 40) {
        mode = FAN_MEDIUM;
    }

    return (FanCommand){.fanMode = mode, .dutyCycle = duty};
}
