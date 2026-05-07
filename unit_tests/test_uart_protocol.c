#include "unity.h"

#include <stdio.h>
#include <string.h>

#include "uart_protocol.h"

static bool parse_command(const char *text, UartCommand *command, char *error, size_t error_size)
{
    char line[64];
    snprintf(line, sizeof(line), "%s", text);
    return uart_protocol_parse_command(line, command, error, error_size);
}

void test_uart_protocol_parses_status_commands(void)
{
    UartCommand command;
    char error[48];

    TEST_ASSERT_TRUE(parse_command("GET_STATUS", &command, error, sizeof(error)));
    TEST_ASSERT_EQUAL_INT(UART_CMD_GET_STATUS, command.type);

    TEST_ASSERT_TRUE(parse_command(" CLEAR_FAULT\r\n", &command, error, sizeof(error)));
    TEST_ASSERT_EQUAL_INT(UART_CMD_CLEAR_FAULT, command.type);
}

void test_uart_protocol_parses_numeric_commands(void)
{
    UartCommand command;
    char error[48];

    TEST_ASSERT_TRUE(parse_command("SET_TEMP:80.5", &command, error, sizeof(error)));
    TEST_ASSERT_EQUAL_INT(UART_CMD_SET_TEMP, command.type);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 80.5f, command.floatValue);

    TEST_ASSERT_TRUE(parse_command("SET_CURRENT:2.1", &command, error, sizeof(error)));
    TEST_ASSERT_EQUAL_INT(UART_CMD_SET_CURRENT, command.type);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 2.1f, command.floatValue);

    TEST_ASSERT_TRUE(parse_command("SET_RPM:1200", &command, error, sizeof(error)));
    TEST_ASSERT_EQUAL_INT(UART_CMD_SET_RPM, command.type);
    TEST_ASSERT_EQUAL_INT(1200, command.intValue);
}

void test_uart_protocol_parses_boolean_commands(void)
{
    UartCommand command;
    char error[48];

    TEST_ASSERT_TRUE(parse_command("SET_SENSOR_VALID:0", &command, error, sizeof(error)));
    TEST_ASSERT_EQUAL_INT(UART_CMD_SET_SENSOR_VALID, command.type);
    TEST_ASSERT_FALSE(command.boolValue);

    TEST_ASSERT_TRUE(parse_command("USE_ADC_INPUT:1", &command, error, sizeof(error)));
    TEST_ASSERT_EQUAL_INT(UART_CMD_USE_ADC_INPUT, command.type);
    TEST_ASSERT_TRUE(command.boolValue);
}

void test_uart_protocol_rejects_invalid_commands(void)
{
    UartCommand command;
    char error[48];

    TEST_ASSERT_FALSE(parse_command("", &command, error, sizeof(error)));
    TEST_ASSERT_EQUAL_STRING("EMPTY_COMMAND", error);

    TEST_ASSERT_FALSE(parse_command("SET_TEMP:not_a_number", &command, error, sizeof(error)));
    TEST_ASSERT_EQUAL_STRING("UNKNOWN_OR_INVALID_COMMAND", error);

    TEST_ASSERT_FALSE(parse_command("SET_SENSOR_VALID:2", &command, error, sizeof(error)));
    TEST_ASSERT_EQUAL_STRING("UNKNOWN_OR_INVALID_COMMAND", error);

    TEST_ASSERT_FALSE(parse_command("BOGUS", &command, error, sizeof(error)));
    TEST_ASSERT_EQUAL_STRING("UNKNOWN_OR_INVALID_COMMAND", error);
}

void test_uart_protocol_formats_status_response(void)
{
    char buffer[160];
    const SensorInput input = {
        .temperature = 80.0f,
        .current = 0.5f,
        .rpm = 1200,
        .sensorValid = true,
        .useAdcInput = false,
    };
    const SystemStatus status = {
        .fanMode = FAN_MEDIUM,
        .dutyCycle = 70,
        .fault = FAULT_NONE,
        .state = STATE_NORMAL,
    };

    uart_protocol_format_status(buffer, sizeof(buffer), &input, &status);

    TEST_ASSERT_EQUAL_STRING("STATUS,TEMP=80.0,CURRENT=0.5,RPM=1200,FAN=MEDIUM,DUTY=70,FAULT=NONE,STATE=NORMAL", buffer);
}

void test_uart_protocol_formats_error_response(void)
{
    char buffer[48];

    uart_protocol_format_error(buffer, sizeof(buffer), "UNKNOWN_OR_INVALID_COMMAND");

    TEST_ASSERT_EQUAL_STRING("ERROR,REASON=UNKNOWN_OR_INVALID_COMMAND", buffer);
}
