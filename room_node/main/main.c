#include "../include/room_config.h"
#include "../include/sample_led.h"
#include "esp_log.h"

static const char *TAG = "room_node";

void app_main(void)
{
    ESP_LOGI(TAG, "Starting room %s (ID %d)", COCIS_ROOM_NAME, COCIS_ROOM_ID);
    exported_run_led_task();
}
