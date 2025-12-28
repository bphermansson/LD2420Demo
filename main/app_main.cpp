#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <drivers/radar.h>

static const char *TAG = "MAIN";

extern "C" void app_main() {
    ESP_LOGI(TAG, "Startar LD2420 Demo...");

    // Initiera drivern
    ld2420_driver_init();

    // Starta tasken
    xTaskCreate(ld2420_task, "ld2420_task", 4096, NULL, 5, NULL);
}