#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "ecu_fan_control";

void app_main(void)
{
    ESP_LOGI(TAG, "ECU Fan Control firmware baseline started");
    ESP_LOGI(TAG, "ESP-IDF + FreeRTOS project skeleton is ready");

    while (true) {
        ESP_LOGI(TAG, "Heartbeat");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
