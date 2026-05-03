#include <stdbool.h>
#include <stdint.h>

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "diagnostics.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_err.h"
#include "esp_log.h"
#include "fan_control.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "system_types.h"

#define TEMP_ADC_CHANNEL ADC_CHANNEL_6
#define TEMP_ADC_GPIO GPIO_NUM_34
#define BUTTON_GPIO GPIO_NUM_4
#define LED_PWM_GPIO GPIO_NUM_26

#define ADC_RAW_MAX 4095
#define SIM_TEMP_MAX_C 100.0f
#define SIM_CURRENT_A 0.5f
#define SIM_RPM 1200
#define STATUS_LOG_PERIOD_MS 500U

#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_CHANNEL LEDC_CHANNEL_0
#define LEDC_DUTY_RES LEDC_TIMER_10_BIT
#define LEDC_DUTY_RES_BITS 10U
#define LEDC_MAX_DUTY ((1U << LEDC_DUTY_RES_BITS) - 1U)
#define LEDC_FREQUENCY_HZ 5000

static const char *TAG = "ecu_fan_control";

static adc_oneshot_unit_handle_t s_adc1_handle;

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

static void set_led_duty_percent(int duty_percent)
{
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, percent_to_ledc_duty(duty_percent)));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
}

void app_main(void)
{
    ESP_LOGI(TAG, "ECU Fan Control Phase 3 core logic POC started");
    ESP_LOGI(TAG, "ADC: GPIO%d, Button: GPIO%d, LED PWM: GPIO%d", TEMP_ADC_GPIO, BUTTON_GPIO, LED_PWM_GPIO);

    configure_adc();
    configure_button();
    configure_led_pwm();

    DiagnosticsContext diagnostics;
    diagnostics_init(&diagnostics);

    TickType_t last_wake = xTaskGetTickCount();
    uint32_t elapsed_since_log_ms = 0;

    while (true) {
        int adc_raw = 0;
        ESP_ERROR_CHECK(adc_oneshot_read(s_adc1_handle, TEMP_ADC_CHANNEL, &adc_raw));

        const int pot_percent = adc_raw_to_percent(adc_raw);
        const float temp_sim_c = adc_raw_to_temperature_c(adc_raw);
        const bool button_pressed = gpio_get_level(BUTTON_GPIO) == 0;
        const SensorInput input = {
            .temperature = temp_sim_c,
            .current = SIM_CURRENT_A,
            .rpm = SIM_RPM,
            .sensorValid = !button_pressed,
            .useAdcInput = true,
        };

        const FanCommand normal_command = fan_control_calculate(input.temperature);
        const SystemStatus status = diagnostics_update(&diagnostics,
                                                       &input,
                                                       normal_command,
                                                       DIAGNOSTICS_CONTROL_CYCLE_MS);

        set_led_duty_percent(status.dutyCycle);

        elapsed_since_log_ms += DIAGNOSTICS_CONTROL_CYCLE_MS;
        if (elapsed_since_log_ms >= STATUS_LOG_PERIOD_MS) {
            elapsed_since_log_ms = 0;

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
        }

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(DIAGNOSTICS_CONTROL_CYCLE_MS));
    }
}
