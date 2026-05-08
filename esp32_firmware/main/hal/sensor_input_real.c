#include "sensor_input_real.h"

#include <stdint.h>

#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_err.h"
#include "esp_log.h"
#include "system_types.h"

#define LM35_ADC_CHANNEL ADC_CHANNEL_6
#define LM35_ADC_RAW_MAX 4095.0f
#define LM35_ADC_REF_VOLTAGE_V 3.3f
#define LM35_VOLTS_PER_DEGREE_C 0.01f

#define INA219_I2C_PORT I2C_NUM_0
#define INA219_SDA_GPIO GPIO_NUM_21
#define INA219_SCL_GPIO GPIO_NUM_22
#define INA219_I2C_ADDRESS 0x40
#define INA219_I2C_SPEED_HZ 100000
#define INA219_I2C_TIMEOUT_MS 20

#define INA219_REG_CONFIG 0x00
#define INA219_REG_CURRENT 0x04
#define INA219_REG_CALIBRATION 0x05
#define INA219_CONFIG_CONTINUOUS_32V_320MV_12BIT 0x399F
#define INA219_CALIBRATION_0R1_100UA_LSB 4096
#define INA219_CURRENT_LSB_A 0.0001f

#define REAL_SENSOR_MIN_TEMP_C (-40.0f)
#define REAL_SENSOR_MAX_TEMP_C 150.0f

static const char *TAG = "sensor_real";

static adc_oneshot_unit_handle_t s_adc1Handle;
static i2c_master_bus_handle_t s_i2cBusHandle;
static i2c_master_dev_handle_t s_ina219Handle;
static bool s_adcInitialized;
static bool s_i2cInitialized;
static bool s_readError;
static float s_lastTemperatureC;
static float s_lastCurrentA;
static int s_lastAdcRaw;

static esp_err_t ina219_write_register(uint8_t reg, uint16_t value)
{
    const uint8_t tx_data[3] = {
        reg,
        (uint8_t)(value >> 8),
        (uint8_t)(value & 0xFFU),
    };

    return i2c_master_transmit(s_ina219Handle, tx_data, sizeof(tx_data), INA219_I2C_TIMEOUT_MS);
}

static esp_err_t ina219_read_register(uint8_t reg, int16_t *value)
{
    uint8_t rx_data[2] = {0};
    const esp_err_t err = i2c_master_transmit_receive(s_ina219Handle,
                                                      &reg,
                                                      1,
                                                      rx_data,
                                                      sizeof(rx_data),
                                                      INA219_I2C_TIMEOUT_MS);
    if (err != ESP_OK) {
        return err;
    }

    *value = (int16_t)((rx_data[0] << 8) | rx_data[1]);
    return ESP_OK;
}

static esp_err_t configure_lm35_adc(void)
{
    adc_oneshot_unit_init_cfg_t unit_config = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    esp_err_t err = adc_oneshot_new_unit(&unit_config, &s_adc1Handle);
    if (err != ESP_OK) {
        return err;
    }

    adc_oneshot_chan_cfg_t channel_config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
    };
    err = adc_oneshot_config_channel(s_adc1Handle, LM35_ADC_CHANNEL, &channel_config);
    if (err != ESP_OK) {
        return err;
    }

    s_adcInitialized = true;
    return ESP_OK;
}

static esp_err_t configure_ina219_i2c(void)
{
    i2c_master_bus_config_t bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = INA219_I2C_PORT,
        .sda_io_num = INA219_SDA_GPIO,
        .scl_io_num = INA219_SCL_GPIO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    esp_err_t err = i2c_new_master_bus(&bus_config, &s_i2cBusHandle);
    if (err != ESP_OK) {
        return err;
    }

    i2c_device_config_t device_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = INA219_I2C_ADDRESS,
        .scl_speed_hz = INA219_I2C_SPEED_HZ,
    };

    err = i2c_master_bus_add_device(s_i2cBusHandle, &device_config, &s_ina219Handle);
    if (err != ESP_OK) {
        return err;
    }

    err = ina219_write_register(INA219_REG_CONFIG, INA219_CONFIG_CONTINUOUS_32V_320MV_12BIT);
    if (err != ESP_OK) {
        return err;
    }

    err = ina219_write_register(INA219_REG_CALIBRATION, INA219_CALIBRATION_0R1_100UA_LSB);
    if (err != ESP_OK) {
        return err;
    }

    s_i2cInitialized = true;
    return ESP_OK;
}

static float real_read_temperature(void)
{
    s_readError = false;

    int adc_raw = 0;
    const esp_err_t err = adc_oneshot_read(s_adc1Handle, LM35_ADC_CHANNEL, &adc_raw);
    if (err != ESP_OK) {
        s_readError = true;
        return s_lastTemperatureC;
    }

    s_lastAdcRaw = adc_raw;
    const float voltage_v = ((float)adc_raw * LM35_ADC_REF_VOLTAGE_V) / LM35_ADC_RAW_MAX;
    s_lastTemperatureC = voltage_v / LM35_VOLTS_PER_DEGREE_C;
    return s_lastTemperatureC;
}

static float real_read_current(void)
{
    int16_t raw_current = 0;
    const esp_err_t err = ina219_read_register(INA219_REG_CURRENT, &raw_current);
    if (err != ESP_OK) {
        s_readError = true;
        return s_lastCurrentA;
    }

    s_lastCurrentA = (float)raw_current * INA219_CURRENT_LSB_A;
    return s_lastCurrentA;
}

static int real_read_rpm(void)
{
    return SENSOR_RPM_UNAVAILABLE;
}

static bool real_is_sensor_valid(void)
{
    return s_adcInitialized &&
           s_i2cInitialized &&
           !s_readError &&
           s_lastTemperatureC >= REAL_SENSOR_MIN_TEMP_C &&
           s_lastTemperatureC <= REAL_SENSOR_MAX_TEMP_C;
}

static bool real_is_adc_input_enabled(void)
{
    return true;
}

static const SensorInputHal s_sensorInputRealHal = {
    .read_temperature = real_read_temperature,
    .read_current = real_read_current,
    .read_rpm = real_read_rpm,
    .is_sensor_valid = real_is_sensor_valid,
    .is_adc_input_enabled = real_is_adc_input_enabled,
};

esp_err_t sensor_input_real_init(void)
{
    s_adcInitialized = false;
    s_i2cInitialized = false;
    s_readError = false;
    s_lastTemperatureC = 0.0f;
    s_lastCurrentA = 0.0f;
    s_lastAdcRaw = 0;

    esp_err_t err = configure_lm35_adc();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "LM35 ADC init failed: %s", esp_err_to_name(err));
        return err;
    }

    err = configure_ina219_i2c();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "INA219 I2C init failed: %s", esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG, "Real sensor HAL initialized: LM35 GPIO34, INA219 I2C 0x%02X, RPM unavailable", INA219_I2C_ADDRESS);
    return ESP_OK;
}

const SensorInputHal *sensor_input_real_get_hal(void)
{
    return &s_sensorInputRealHal;
}

int sensor_input_real_get_adc_raw(void)
{
    return s_lastAdcRaw;
}
