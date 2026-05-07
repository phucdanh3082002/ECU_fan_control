#include "diagnostics.h"

#define SENSOR_MIN_TEMP_C (-40.0f)
#define SENSOR_MAX_TEMP_C 150.0f
#define OVER_TEMP_THRESHOLD_C 100.0f
#define OVER_CURRENT_THRESHOLD_A 2.0f

static SystemStatus normal_status(FanCommand command)
{
    return (SystemStatus){
        .fanMode = command.fanMode,
        .dutyCycle = command.dutyCycle,
        .fault = FAULT_NONE,
        .state = STATE_NORMAL,
    };
}

static SystemStatus fault_status(FaultCode fault)
{
    switch (fault) {
    case FAULT_SENSOR:
        return (SystemStatus){.fanMode = FAN_HIGH, .dutyCycle = 100, .fault = fault, .state = STATE_SAFE_MODE};
    case FAULT_OVER_TEMP:
        return (SystemStatus){.fanMode = FAN_HIGH, .dutyCycle = 100, .fault = fault, .state = STATE_FAULT_MODE};
    case FAULT_OVER_CURRENT:
    case FAULT_FAN_STALL:
        return (SystemStatus){.fanMode = FAN_OFF, .dutyCycle = 0, .fault = fault, .state = STATE_FAULT_MODE};
    case FAULT_NONE:
    default:
        return (SystemStatus){.fanMode = FAN_OFF, .dutyCycle = 0, .fault = FAULT_NONE, .state = STATE_NORMAL};
    }
}

static FaultCode detect_active_fault(DiagnosticsContext *context,
                                     const SensorInput *input,
                                     FanCommand normalCommand,
                                     uint32_t cycleMs)
{
    if (!input->sensorValid || input->temperature < SENSOR_MIN_TEMP_C || input->temperature > SENSOR_MAX_TEMP_C) {
        return FAULT_SENSOR;
    }

    if (input->current > OVER_CURRENT_THRESHOLD_A) {
        return FAULT_OVER_CURRENT;
    }

    if (input->temperature >= OVER_TEMP_THRESHOLD_C) {
        return FAULT_OVER_TEMP;
    }

    const bool rpmFeedbackAvailable = input->rpm >= 0;
    if (rpmFeedbackAvailable && normalCommand.dutyCycle > 0 && input->rpm == 0) {
        context->fanStallElapsedMs += cycleMs;
        if (context->fanStallElapsedMs >= DIAGNOSTICS_FAN_STALL_TIMEOUT_MS) {
            return FAULT_FAN_STALL;
        }
    } else {
        context->fanStallElapsedMs = 0;
    }

    return FAULT_NONE;
}

void diagnostics_init(DiagnosticsContext *context)
{
    context->fanStallElapsedMs = 0;
    context->recoveryCycles = 0;
    context->latchedFault = FAULT_NONE;
}

SystemStatus diagnostics_update(DiagnosticsContext *context,
                                const SensorInput *input,
                                FanCommand normalCommand,
                                uint32_t cycleMs)
{
    const FaultCode activeFault = detect_active_fault(context, input, normalCommand, cycleMs);

    if (activeFault != FAULT_NONE) {
        context->latchedFault = activeFault;
        context->recoveryCycles = 0;
        return fault_status(activeFault);
    }

    if (context->latchedFault != FAULT_NONE) {
        context->recoveryCycles++;
        if (context->recoveryCycles < DIAGNOSTICS_RECOVERY_CYCLES_REQUIRED) {
            return fault_status(context->latchedFault);
        }

        context->latchedFault = FAULT_NONE;
        context->recoveryCycles = 0;
    }

    return normal_status(normalCommand);
}
