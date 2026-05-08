#include "fan_control.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define FAN_ON_TEMP_C    20.0f
#define FAN_FULL_TEMP_C  70.0f
#define FAN_DUTY_MIN     35
#define FAN_DUTY_MAX     100

#define FAN_STARTUP_DUTY     100
#define FAN_STARTUP_TIME_MS  1200U

static TickType_t s_fanStartedTick = 0;
static bool s_fanStartupActive = false;
static bool s_prevFanOff = true;

FanCommand fan_control_calculate(float temperature_c)
{
    if (temperature_c < FAN_ON_TEMP_C) {
        s_prevFanOff = true;
        s_fanStartupActive = false;
        return (FanCommand){.fanMode = FAN_OFF, .dutyCycle = 0};
    }

    if (temperature_c >= FAN_FULL_TEMP_C) {
        if (s_prevFanOff) {
            s_fanStartupActive = true;
            s_fanStartedTick = xTaskGetTickCount();
            s_prevFanOff = false;
            return (FanCommand){.fanMode = FAN_HIGH, .dutyCycle = FAN_STARTUP_DUTY};
        }
        s_prevFanOff = false;

        if (s_fanStartupActive) {
            TickType_t elapsed = xTaskGetTickCount() - s_fanStartedTick;
            if (elapsed < pdMS_TO_TICKS(FAN_STARTUP_TIME_MS)) {
                return (FanCommand){.fanMode = FAN_HIGH, .dutyCycle = FAN_STARTUP_DUTY};
            }
            s_fanStartupActive = false;
        }

        return (FanCommand){.fanMode = FAN_HIGH, .dutyCycle = FAN_DUTY_MAX};
    }

    float ratio = (temperature_c - FAN_ON_TEMP_C) / (FAN_FULL_TEMP_C - FAN_ON_TEMP_C);
    int duty = FAN_DUTY_MIN + (int)(ratio * (FAN_DUTY_MAX - FAN_DUTY_MIN));

    if (duty < FAN_DUTY_MIN) {
        duty = FAN_DUTY_MIN;
    }

    FanMode mode = FAN_LOW;
    if (duty >= 70) {
        mode = FAN_HIGH;
    } else if (duty >= 40) {
        mode = FAN_MEDIUM;
    }

    if (s_prevFanOff) {
        s_fanStartupActive = true;
        s_fanStartedTick = xTaskGetTickCount();
        s_prevFanOff = false;
        return (FanCommand){.fanMode = mode, .dutyCycle = FAN_STARTUP_DUTY};
    }

    if (s_fanStartupActive) {
        TickType_t elapsed = xTaskGetTickCount() - s_fanStartedTick;
        if (elapsed < pdMS_TO_TICKS(FAN_STARTUP_TIME_MS)) {
            return (FanCommand){.fanMode = mode, .dutyCycle = FAN_STARTUP_DUTY};
        }
        s_fanStartupActive = false;
    }

    s_prevFanOff = false;
    return (FanCommand){.fanMode = mode, .dutyCycle = duty};
}
