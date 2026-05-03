#ifndef SYSTEM_TYPES_H
#define SYSTEM_TYPES_H

#include <stdbool.h>

typedef enum {
    FAN_OFF,
    FAN_LOW,
    FAN_MEDIUM,
    FAN_HIGH
} FanMode;

typedef enum {
    FAULT_NONE,
    FAULT_SENSOR,
    FAULT_OVER_TEMP,
    FAULT_OVER_CURRENT,
    FAULT_FAN_STALL
} FaultCode;

typedef enum {
    STATE_NORMAL,
    STATE_SAFE_MODE,
    STATE_FAULT_MODE
} SystemState;

typedef struct {
    float temperature;
    float current;
    int rpm;
    bool sensorValid;
    bool useAdcInput;
} SensorInput;

typedef struct {
    FanMode fanMode;
    int dutyCycle;
    FaultCode fault;
    SystemState state;
} SystemStatus;

const char *fan_mode_to_string(FanMode mode);
const char *fault_code_to_string(FaultCode fault);
const char *system_state_to_string(SystemState state);

#endif
