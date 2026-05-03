#include "system_types.h"

const char *fan_mode_to_string(FanMode mode)
{
    switch (mode) {
    case FAN_OFF:
        return "OFF";
    case FAN_LOW:
        return "LOW";
    case FAN_MEDIUM:
        return "MEDIUM";
    case FAN_HIGH:
        return "HIGH";
    default:
        return "UNKNOWN";
    }
}

const char *fault_code_to_string(FaultCode fault)
{
    switch (fault) {
    case FAULT_NONE:
        return "NONE";
    case FAULT_SENSOR:
        return "SENSOR_FAULT";
    case FAULT_OVER_TEMP:
        return "OVER_TEMPERATURE";
    case FAULT_OVER_CURRENT:
        return "OVER_CURRENT";
    case FAULT_FAN_STALL:
        return "FAN_STALL";
    default:
        return "UNKNOWN";
    }
}

const char *system_state_to_string(SystemState state)
{
    switch (state) {
    case STATE_NORMAL:
        return "NORMAL";
    case STATE_SAFE_MODE:
        return "SAFE_MODE";
    case STATE_FAULT_MODE:
        return "FAULT_MODE";
    default:
        return "UNKNOWN";
    }
}
