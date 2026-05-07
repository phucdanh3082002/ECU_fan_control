#include "uart_protocol.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *trim_ascii_whitespace(char *text)
{
    while (*text == ' ' || *text == '\t' || *text == '\r' || *text == '\n') {
        text++;
    }

    char *end = text + strlen(text);
    while (end > text && (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\r' || end[-1] == '\n')) {
        end--;
    }
    *end = '\0';

    return text;
}

static void set_error(char *error, size_t errorSize, const char *message)
{
    if (errorSize == 0) {
        return;
    }

    snprintf(error, errorSize, "%s", message);
}

static bool parse_float_value(const char *text, float *value)
{
    char *end = NULL;
    errno = 0;
    const float parsed = strtof(text, &end);

    if (errno != 0 || end == text) {
        return false;
    }

    end = trim_ascii_whitespace(end);
    if (*end != '\0') {
        return false;
    }

    *value = parsed;
    return true;
}

static bool parse_int_value(const char *text, int *value)
{
    char *end = NULL;
    errno = 0;
    const long parsed = strtol(text, &end, 10);

    if (errno != 0 || end == text) {
        return false;
    }

    end = trim_ascii_whitespace(end);
    if (*end != '\0') {
        return false;
    }

    *value = (int)parsed;
    return true;
}

static bool parse_bool_value(const char *text, bool *value)
{
    int parsed = 0;
    if (!parse_int_value(text, &parsed)) {
        return false;
    }

    if (parsed == 0) {
        *value = false;
        return true;
    }

    if (parsed == 1) {
        *value = true;
        return true;
    }

    return false;
}

static bool parse_prefixed_float(const char *line, const char *prefix, UartCommandType type, UartCommand *command)
{
    const size_t prefix_len = strlen(prefix);
    if (strncmp(line, prefix, prefix_len) != 0) {
        return false;
    }

    float value = 0.0f;
    if (!parse_float_value(line + prefix_len, &value)) {
        return false;
    }

    command->type = type;
    command->floatValue = value;
    return true;
}

static bool parse_prefixed_int(const char *line, const char *prefix, UartCommandType type, UartCommand *command)
{
    const size_t prefix_len = strlen(prefix);
    if (strncmp(line, prefix, prefix_len) != 0) {
        return false;
    }

    int value = 0;
    if (!parse_int_value(line + prefix_len, &value)) {
        return false;
    }

    command->type = type;
    command->intValue = value;
    return true;
}

static bool parse_prefixed_bool(const char *line, const char *prefix, UartCommandType type, UartCommand *command)
{
    const size_t prefix_len = strlen(prefix);
    if (strncmp(line, prefix, prefix_len) != 0) {
        return false;
    }

    bool value = false;
    if (!parse_bool_value(line + prefix_len, &value)) {
        return false;
    }

    command->type = type;
    command->boolValue = value;
    return true;
}

bool uart_protocol_parse_command(char *line, UartCommand *command, char *error, size_t errorSize)
{
    const char *trimmed = trim_ascii_whitespace(line);

    if (*trimmed == '\0') {
        set_error(error, errorSize, "EMPTY_COMMAND");
        return false;
    }

    memset(command, 0, sizeof(*command));

    if (strcmp(trimmed, "GET_STATUS") == 0) {
        command->type = UART_CMD_GET_STATUS;
        return true;
    }

    if (strcmp(trimmed, "CLEAR_FAULT") == 0) {
        command->type = UART_CMD_CLEAR_FAULT;
        return true;
    }

    if (parse_prefixed_float(trimmed, "SET_TEMP:", UART_CMD_SET_TEMP, command)) {
        return true;
    }

    if (parse_prefixed_float(trimmed, "SET_CURRENT:", UART_CMD_SET_CURRENT, command)) {
        return true;
    }

    if (parse_prefixed_int(trimmed, "SET_RPM:", UART_CMD_SET_RPM, command)) {
        return true;
    }

    if (parse_prefixed_bool(trimmed, "SET_SENSOR_VALID:", UART_CMD_SET_SENSOR_VALID, command)) {
        return true;
    }

    if (parse_prefixed_bool(trimmed, "USE_ADC_INPUT:", UART_CMD_USE_ADC_INPUT, command)) {
        return true;
    }

    set_error(error, errorSize, "UNKNOWN_OR_INVALID_COMMAND");
    return false;
}

void uart_protocol_format_status(char *buffer, size_t bufferSize, const SensorInput *input, const SystemStatus *status)
{
    snprintf(buffer,
             bufferSize,
             "STATUS,TEMP=%.1f,CURRENT=%.1f,RPM=%d,FAN=%s,DUTY=%d,FAULT=%s,STATE=%s",
             input->temperature,
             input->current,
             input->rpm,
             fan_mode_to_string(status->fanMode),
             status->dutyCycle,
             fault_code_to_string(status->fault),
             system_state_to_string(status->state));
}

void uart_protocol_format_error(char *buffer, size_t bufferSize, const char *reason)
{
    snprintf(buffer, bufferSize, "ERROR,REASON=%s", reason);
}
