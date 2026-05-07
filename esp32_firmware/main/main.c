#include "app_tasks.h"

#include "esp_log.h"

static const char *TAG = "ecu_fan_control";

void app_main(void)
{
    ESP_LOGI(TAG, "ECU Fan Control Phase 10 HAL simulation started");
    app_tasks_start();
}
