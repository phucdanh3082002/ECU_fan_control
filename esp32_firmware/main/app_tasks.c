#include "app_tasks.h"

#include <stdbool.h>
#include <stdint.h>

#include "diagnostics.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "driver/uart.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_err.h"
#include "esp_log.h"
#include "fan_control.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "hal/sensor_input.h"
#if ECU_SENSOR_HAL_SIM
#include "hal/sensor_input_sim.h"
#elif ECU_SENSOR_HAL_REAL
#include "hal/sensor_input_real.h"
#else
#error "Unsupported ECU_SENSOR_HAL selection"
#endif
#include <string.h>
#include "system_types.h"
#include "uart_protocol.h"

#define TEMP_ADC_CHANNEL ADC_CHANNEL_6
#define TEMP_ADC_GPIO GPIO_NUM_34
#define BUTTON_GPIO GPIO_NUM_4
#define LED_PWM_GPIO GPIO_NUM_26

#define ADC_RAW_MAX 4095
#define SIM_TEMP_MAX_C 100.0f

#define ANALOG_INPUT_PERIOD_MS 100U
#define BUTTON_INPUT_PERIOD_MS 50U
#define SENSOR_READ_PERIOD_MS 100U
#define FAN_CONTROL_PERIOD_MS 100U
#define DIAGNOSTICS_PERIOD_MS 100U
#define PWM_OUTPUT_PERIOD_MS 100U
#define STATUS_REPORT_PERIOD_MS 500U

#define UART_PORT UART_NUM_0
#define UART_BAUD_RATE 115200
#define UART_RX_BUFFER_SIZE 1024
#define UART_TX_BUFFER_SIZE 1024
#define UART_COMMAND_BUFFER_SIZE 96
#define UART_RESPONSE_BUFFER_SIZE 160
#define UART_COMMAND_POLL_MS 50U
#define UART_COMMAND_IDLE_PARSE_MS 300U
#define UART_COMMAND_APPLY_DELAY_MS 350U

#define UART_COMMAND_TASK_PRIORITY 5
#define DIAGNOSTICS_TASK_PRIORITY 4
#define FAN_CONTROL_TASK_PRIORITY 4
#define SENSOR_READ_TASK_PRIORITY 4
#define ANALOG_INPUT_TASK_PRIORITY 3
#define BUTTON_INPUT_TASK_PRIORITY 3
#define PWM_OUTPUT_TASK_PRIORITY 2
#define STATUS_REPORT_TASK_PRIORITY 2

#define TASK_STACK_SMALL 2048
#define TASK_STACK_MEDIUM 3072
#define TASK_STACK_LARGE 4096

#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_CHANNEL LEDC_CHANNEL_0
#define LEDC_DUTY_RES LEDC_TIMER_10_BIT
#define LEDC_DUTY_RES_BITS 10U
#define LEDC_MAX_DUTY ((1U << LEDC_DUTY_RES_BITS) - 1U)
#define LEDC_FREQUENCY_HZ 5000

static const char *TAG = "app_tasks";

static SemaphoreHandle_t systemMutex;
#if ECU_SENSOR_HAL_SIM
static adc_oneshot_unit_handle_t s_adc1_handle;
#endif
static DiagnosticsContext s_diagnostics;
static const SensorInputHal *s_sensorInputHal;

static SensorInput g_sensorInput = {
    .temperature = 0.0f,
    .current = 0.5f,
    .rpm = 1200,
    .sensorValid = true,
    .useAdcInput = true,
};

static SystemStatus g_systemStatus = {
    .fanMode = FAN_OFF,
    .dutyCycle = 0,
    .fault = FAULT_NONE,
    .state = STATE_NORMAL,
};

static FanCommand g_normalFanCommand = {
    .fanMode = FAN_OFF,
    .dutyCycle = 0,
};

static int g_adcRaw;
static int g_potPercent;
static bool g_buttonPressed;
static bool g_clearFaultRequested;

#if ECU_SENSOR_HAL_SIM
static int clamp_adc_raw(int raw)
{
    if (raw < 0) {
        return 0;
    }

    if (raw > ADC_RAW_MAX) {
        return ADC_RAW_MAX;
    }

    return raw;
}

static int adc_raw_to_percent(int raw)
{
    raw = clamp_adc_raw(raw);
    return (raw * 100 + (ADC_RAW_MAX / 2)) / ADC_RAW_MAX;
}

static float adc_raw_to_temperature_c(int raw)
{
    raw = clamp_adc_raw(raw);
    return ((float)raw * SIM_TEMP_MAX_C) / (float)ADC_RAW_MAX;
}
#endif

static uint32_t percent_to_ledc_duty(int percent)
{
    if (percent < 0) {
        percent = 0;
    }

    if (percent > 100) {
        percent = 100;
    }

    return (uint32_t)((percent * (int)LEDC_MAX_DUTY + 50) / 100);
}

#if ECU_SENSOR_HAL_SIM
static void configure_adc(void)
{
    adc_oneshot_unit_init_cfg_t unit_config = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit_config, &s_adc1_handle));

    adc_oneshot_chan_cfg_t channel_config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(s_adc1_handle, TEMP_ADC_CHANNEL, &channel_config));
}

static void configure_button(void)
{
    gpio_config_t button_config = {
        .pin_bit_mask = 1ULL << BUTTON_GPIO,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&button_config));
}
#endif

static void configure_led_pwm(void)
{
    ledc_timer_config_t timer_config = {
        .speed_mode = LEDC_MODE,
        .duty_resolution = LEDC_DUTY_RES,
        .timer_num = LEDC_TIMER,
        .freq_hz = LEDC_FREQUENCY_HZ,
        .clk_cfg = LEDC_AUTO_CLK,
        .deconfigure = false,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer_config));

    ledc_channel_config_t channel_config = {
        .gpio_num = LED_PWM_GPIO,
        .speed_mode = LEDC_MODE,
        .channel = LEDC_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER,
        .duty = 0,
        .hpoint = 0,
        .sleep_mode = LEDC_SLEEP_MODE_NO_ALIVE_NO_PD,
        .flags.output_invert = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&channel_config));
}

static void configure_uart(void)
{
    const uart_config_t uart_config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 0,
        .source_clk = UART_SCLK_DEFAULT,
    };

    ESP_ERROR_CHECK(uart_param_config(UART_PORT, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    if (!uart_is_driver_installed(UART_PORT)) {
        ESP_ERROR_CHECK(uart_driver_install(UART_PORT, UART_RX_BUFFER_SIZE, UART_TX_BUFFER_SIZE, 0, NULL, 0));
    }
}

static void set_led_duty_percent(int duty_percent)
{
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, percent_to_ledc_duty(duty_percent)));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
}

static void lock_system_state(void)
{
    ESP_ERROR_CHECK(systemMutex != NULL ? ESP_OK : ESP_ERR_INVALID_STATE);
    xSemaphoreTake(systemMutex, portMAX_DELAY);
}

static void unlock_system_state(void)
{
    xSemaphoreGive(systemMutex);
}

static SensorInput read_sensor_input_from_hal(void)
{
    ESP_ERROR_CHECK(s_sensorInputHal != NULL ? ESP_OK : ESP_ERR_INVALID_STATE);
    return sensor_input_hal_read(s_sensorInputHal);
}

static void refresh_sensor_input_from_hal(void)
{
    g_sensorInput = read_sensor_input_from_hal();
}

static void create_task(TaskFunction_t taskFunction, const char *name, uint32_t stackDepth, UBaseType_t priority)
{
    const BaseType_t result = xTaskCreate(taskFunction, name, stackDepth, NULL, priority, NULL);
    ESP_ERROR_CHECK(result == pdPASS ? ESP_OK : ESP_ERR_NO_MEM);
}

static void write_uart_line(const char *line)
{
    uart_write_bytes(UART_PORT, line, strlen(line));
    uart_write_bytes(UART_PORT, "\r\n", 2);
}

static void send_status_response(void)
{
    SensorInput input;
    SystemStatus status;
    char response[UART_RESPONSE_BUFFER_SIZE];

    lock_system_state();
    input = g_sensorInput;
    status = g_systemStatus;
    unlock_system_state();

    uart_protocol_format_status(response, sizeof(response), &input, &status);
    write_uart_line(response);
}

static void send_error_response(const char *reason)
{
    char response[UART_RESPONSE_BUFFER_SIZE];

    uart_protocol_format_error(response, sizeof(response), reason);
    write_uart_line(response);
}

static void apply_uart_command(const UartCommand *command)
{
    lock_system_state();

    switch (command->type) {
    case UART_CMD_SET_TEMP:
#if ECU_SENSOR_HAL_SIM
        sensor_input_sim_set_uart_temperature(command->floatValue);
        sensor_input_sim_set_use_adc_input(false);
        ESP_LOGI(TAG, "UART: SET_TEMP=%.1f, ADC disabled", command->floatValue);
#else
        ESP_LOGW(TAG, "UART: SET_TEMP ignored in real sensor HAL");
#endif
        break;
    case UART_CMD_SET_CURRENT:
#if ECU_SENSOR_HAL_SIM
        sensor_input_sim_set_current(command->floatValue);
        ESP_LOGI(TAG, "UART: SET_CURRENT=%.1f", command->floatValue);
#else
        ESP_LOGW(TAG, "UART: SET_CURRENT ignored in real sensor HAL");
#endif
        break;
    case UART_CMD_SET_RPM:
#if ECU_SENSOR_HAL_SIM
        sensor_input_sim_set_rpm(command->intValue);
        ESP_LOGI(TAG, "UART: SET_RPM=%d", command->intValue);
#else
        ESP_LOGW(TAG, "UART: SET_RPM ignored because tach input is not configured");
#endif
        break;
    case UART_CMD_SET_SENSOR_VALID:
#if ECU_SENSOR_HAL_SIM
        sensor_input_sim_set_uart_sensor_valid(command->boolValue);
        ESP_LOGI(TAG, "UART: SET_SENSOR_VALID=%d", command->boolValue);
#else
        ESP_LOGW(TAG, "UART: SET_SENSOR_VALID ignored in real sensor HAL");
#endif
        break;
    case UART_CMD_USE_ADC_INPUT:
#if ECU_SENSOR_HAL_SIM
        sensor_input_sim_set_use_adc_input(command->boolValue);
        ESP_LOGI(TAG, "UART: USE_ADC_INPUT=%d", command->boolValue);
#else
        ESP_LOGW(TAG, "UART: USE_ADC_INPUT ignored in real sensor HAL");
#endif
        break;
    case UART_CMD_CLEAR_FAULT:
        g_clearFaultRequested = true;
        ESP_LOGI(TAG, "UART: CLEAR_FAULT requested");
        break;
    case UART_CMD_GET_STATUS:
        ESP_LOGI(TAG, "UART: GET_STATUS");
        break;
    default:
        break;
    }

    refresh_sensor_input_from_hal();

    unlock_system_state();
}

static void process_uart_command_line(char *command_buffer)
{
    ESP_LOGI(TAG, "UART RX: [%s]", command_buffer);

    UartCommand command;
    char error[48];
    if (!uart_protocol_parse_command(command_buffer, &command, error, sizeof(error))) {
        ESP_LOGI(TAG, "UART ERROR: %s", error);
        send_error_response(error);
        return;
    }

    apply_uart_command(&command);

    if (command.type != UART_CMD_GET_STATUS) {
        vTaskDelay(pdMS_TO_TICKS(UART_COMMAND_APPLY_DELAY_MS));
    }
    send_status_response();
}

#if ECU_SENSOR_HAL_SIM
static void analog_input_task(void *parameter)
{
    (void)parameter;
    TickType_t last_wake = xTaskGetTickCount();

    while (true) {
        int adc_raw = 0;
        ESP_ERROR_CHECK(adc_oneshot_read(s_adc1_handle, TEMP_ADC_CHANNEL, &adc_raw));

        const int pot_percent = adc_raw_to_percent(adc_raw);
        const float temperature_c = adc_raw_to_temperature_c(adc_raw);

        lock_system_state();
        g_adcRaw = adc_raw;
        g_potPercent = pot_percent;
        sensor_input_sim_set_adc_temperature(temperature_c);
        refresh_sensor_input_from_hal();
        unlock_system_state();

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(ANALOG_INPUT_PERIOD_MS));
    }
}

static void button_input_task(void *parameter)
{
    (void)parameter;
    TickType_t last_wake = xTaskGetTickCount();

    while (true) {
        const bool button_pressed = gpio_get_level(BUTTON_GPIO) == 0;

        lock_system_state();
        g_buttonPressed = button_pressed;
        sensor_input_sim_set_button_pressed(button_pressed);
        refresh_sensor_input_from_hal();
        unlock_system_state();

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(BUTTON_INPUT_PERIOD_MS));
    }
}
#endif

#if ECU_SENSOR_HAL_REAL
static void sensor_read_task(void *parameter)
{
    (void)parameter;
    TickType_t last_wake = xTaskGetTickCount();

    while (true) {
        const SensorInput input = read_sensor_input_from_hal();

        lock_system_state();
        g_sensorInput = input;
        unlock_system_state();

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(SENSOR_READ_PERIOD_MS));
    }
}
#endif

static void fan_control_task(void *parameter)
{
    (void)parameter;
    TickType_t last_wake = xTaskGetTickCount();

    while (true) {
        float temperature_c = 0.0f;

        lock_system_state();
        temperature_c = g_sensorInput.temperature;
        unlock_system_state();

        const FanCommand command = fan_control_calculate(temperature_c);

        lock_system_state();
        g_normalFanCommand = command;
        unlock_system_state();

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(FAN_CONTROL_PERIOD_MS));
    }
}

static void diagnostics_task(void *parameter)
{
    (void)parameter;
    TickType_t last_wake = xTaskGetTickCount();

    while (true) {
        SensorInput input;
        FanCommand normal_command;
        bool clear_fault_requested = false;

        lock_system_state();
        input = g_sensorInput;
        normal_command = g_normalFanCommand;
        clear_fault_requested = g_clearFaultRequested;
        g_clearFaultRequested = false;
        unlock_system_state();

        if (clear_fault_requested) {
            diagnostics_init(&s_diagnostics);
        }

        const SystemStatus status = diagnostics_update(&s_diagnostics,
                                                       &input,
                                                       normal_command,
                                                       DIAGNOSTICS_PERIOD_MS);

        lock_system_state();
        g_systemStatus = status;
        unlock_system_state();

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(DIAGNOSTICS_PERIOD_MS));
    }
}

static void pwm_output_task(void *parameter)
{
    (void)parameter;
    TickType_t last_wake = xTaskGetTickCount();

    while (true) {
        int duty_cycle = 0;

        lock_system_state();
        duty_cycle = g_systemStatus.dutyCycle;
        unlock_system_state();

        set_led_duty_percent(duty_cycle);

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(PWM_OUTPUT_PERIOD_MS));
    }
}

static void status_report_task(void *parameter)
{
    (void)parameter;
    TickType_t last_wake = xTaskGetTickCount();

    while (true) {
        SensorInput input;
        SystemStatus status;
        int adc_raw = 0;
        int pot_percent = 0;
        bool button_pressed = false;

        lock_system_state();
        input = g_sensorInput;
        status = g_systemStatus;
        adc_raw = g_adcRaw;
        pot_percent = g_potPercent;
        button_pressed = g_buttonPressed;
        unlock_system_state();

        ESP_LOGI(TAG,
                 "STATUS,TEMP=%.1f,CURRENT=%.1f,RPM=%d,SENSOR=%d,FAN=%s,DUTY=%d,FAULT=%s,STATE=%s,ADC_RAW=%d,POT=%d%%,BUTTON=%s",
                 input.temperature,
                 input.current,
                 input.rpm,
                 input.sensorValid ? 1 : 0,
                 fan_mode_to_string(status.fanMode),
                 status.dutyCycle,
                 fault_code_to_string(status.fault),
                 system_state_to_string(status.state),
                 adc_raw,
                 pot_percent,
                 button_pressed ? "PRESSED" : "RELEASED");

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(STATUS_REPORT_PERIOD_MS));
    }
}

static void uart_command_task(void *parameter)
{
    (void)parameter;
    char command_buffer[UART_COMMAND_BUFFER_SIZE];
    size_t command_length = 0;
    TickType_t last_rx_tick = xTaskGetTickCount();

    ESP_LOGI(TAG, "UartCommandTask running on UART%d at %d baud", UART_PORT, UART_BAUD_RATE);

    while (true) {
        uint8_t byte = 0;
        const int bytes_read = uart_read_bytes(UART_PORT, &byte, 1, pdMS_TO_TICKS(UART_COMMAND_POLL_MS));

        if (bytes_read <= 0) {
            const TickType_t now = xTaskGetTickCount();
            if (command_length > 0 && (now - last_rx_tick) >= pdMS_TO_TICKS(UART_COMMAND_IDLE_PARSE_MS)) {
                command_buffer[command_length] = '\0';
                command_length = 0;
                process_uart_command_line(command_buffer);
            }
            continue;
        }

        last_rx_tick = xTaskGetTickCount();

        if ((byte == 'n' || byte == 'r') && command_length > 0 && command_buffer[command_length - 1U] == '\\') {
            command_buffer[command_length - 1U] = '\0';
            command_length = 0;
            process_uart_command_line(command_buffer);
            continue;
        }

        if (byte == '\r' || byte == '\n') {
            if (command_length == 0) {
                continue;
            }

            command_buffer[command_length] = '\0';
            command_length = 0;
            process_uart_command_line(command_buffer);
            continue;
        }

        if (command_length < sizeof(command_buffer) - 1U) {
            command_buffer[command_length++] = (char)byte;
        } else {
            command_length = 0;
            send_error_response("COMMAND_TOO_LONG");
        }
    }
}

void app_tasks_start(void)
{
#if ECU_SENSOR_HAL_SIM
    ESP_LOGI(TAG, "Configuring hardware: ADC GPIO%d, Button GPIO%d, LED PWM GPIO%d", TEMP_ADC_GPIO, BUTTON_GPIO, LED_PWM_GPIO);

    configure_adc();
    configure_button();
#elif ECU_SENSOR_HAL_REAL
    ESP_LOGI(TAG, "Configuring real hardware: LM35 GPIO%d, INA219 GPIO21/22, Fan PWM GPIO%d, GPIO27 not connected", TEMP_ADC_GPIO, LED_PWM_GPIO);
#endif
    configure_led_pwm();
    configure_uart();

#if ECU_SENSOR_HAL_SIM
    sensor_input_sim_init();
    s_sensorInputHal = sensor_input_sim_get_hal();
#elif ECU_SENSOR_HAL_REAL
    ESP_ERROR_CHECK(sensor_input_real_init());
    s_sensorInputHal = sensor_input_real_get_hal();
#endif
    g_sensorInput = read_sensor_input_from_hal();

    diagnostics_init(&s_diagnostics);

    systemMutex = xSemaphoreCreateMutex();
    ESP_ERROR_CHECK(systemMutex != NULL ? ESP_OK : ESP_ERR_NO_MEM);

    create_task(uart_command_task, "UartCommandTask", TASK_STACK_MEDIUM, UART_COMMAND_TASK_PRIORITY);
    create_task(diagnostics_task, "DiagnosticsTask", TASK_STACK_MEDIUM, DIAGNOSTICS_TASK_PRIORITY);
    create_task(fan_control_task, "FanControlTask", TASK_STACK_MEDIUM, FAN_CONTROL_TASK_PRIORITY);
#if ECU_SENSOR_HAL_SIM
    create_task(analog_input_task, "AnalogInputTask", TASK_STACK_MEDIUM, ANALOG_INPUT_TASK_PRIORITY);
    create_task(button_input_task, "ButtonInputTask", TASK_STACK_SMALL, BUTTON_INPUT_TASK_PRIORITY);
#elif ECU_SENSOR_HAL_REAL
    create_task(sensor_read_task, "SensorReadTask", TASK_STACK_MEDIUM, SENSOR_READ_TASK_PRIORITY);
#endif
    create_task(pwm_output_task, "PwmOutputTask", TASK_STACK_MEDIUM, PWM_OUTPUT_TASK_PRIORITY);
    create_task(status_report_task, "StatusReportTask", TASK_STACK_LARGE, STATUS_REPORT_TASK_PRIORITY);

#if ECU_SENSOR_HAL_SIM
    ESP_LOGI(TAG, "FreeRTOS tasks started: UART, diagnostics, fan control, analog input, button input, PWM output, status report");
#elif ECU_SENSOR_HAL_REAL
    ESP_LOGI(TAG, "FreeRTOS tasks started: UART, diagnostics, fan control, sensor read, fan PWM output, status report");
#endif
}
