#ifndef UART_PROTOCOL_H
#define UART_PROTOCOL_H

#include <stdbool.h>
#include <stddef.h>

#include "system_types.h"

typedef enum {
    UART_CMD_SET_TEMP,
    UART_CMD_SET_CURRENT,
    UART_CMD_SET_RPM,
    UART_CMD_SET_SENSOR_VALID,
    UART_CMD_USE_ADC_INPUT,
    UART_CMD_CLEAR_FAULT,
    UART_CMD_GET_STATUS,
} UartCommandType;

typedef struct {
    UartCommandType type;
    float floatValue;
    int intValue;
    bool boolValue;
} UartCommand;

bool uart_protocol_parse_command(char *line, UartCommand *command, char *error, size_t errorSize);
void uart_protocol_format_status(char *buffer, size_t bufferSize, const SensorInput *input, const SystemStatus *status);
void uart_protocol_format_error(char *buffer, size_t bufferSize, const char *reason);

#endif
