#ifndef DIAGNOSTICS_H
#define DIAGNOSTICS_H

#include <stdint.h>

#include "fan_control.h"
#include "system_types.h"

#define DIAGNOSTICS_CONTROL_CYCLE_MS 100U
#define DIAGNOSTICS_RECOVERY_CYCLES_REQUIRED 3U
#define DIAGNOSTICS_FAN_STALL_TIMEOUT_MS 1000U

typedef struct {
    uint32_t fanStallElapsedMs;
    uint32_t recoveryCycles;
    FaultCode latchedFault;
} DiagnosticsContext;

void diagnostics_init(DiagnosticsContext *context);
SystemStatus diagnostics_update(DiagnosticsContext *context,
                                const SensorInput *input,
                                FanCommand normalCommand,
                                uint32_t cycleMs);

#endif
