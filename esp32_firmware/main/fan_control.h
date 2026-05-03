#ifndef FAN_CONTROL_H
#define FAN_CONTROL_H

#include "system_types.h"

typedef struct {
    FanMode fanMode;
    int dutyCycle;
} FanCommand;

FanCommand fan_control_calculate(float temperature_c);

#endif
