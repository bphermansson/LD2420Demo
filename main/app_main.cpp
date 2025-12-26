#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define SENSOR_TX_PIN 9   // ESP32 TX (kopplas till sensorns RX)
#define SENSOR_RX_PIN 10  // ESP32 RX (kopplas till sensorns TX)
#define UART_PORT     UART_NUM_1
#define BUF_SIZE      1024
static const char *TAG = "LD2420_RADAR";

void radar_task(void *arg) {
    uint8_t data[BUF_SIZE];
    
    // --- INSTÄLLNINGAR ---
    const int MIN_GRANS = 1500;   // Ignorera allt under 1.5 meter
    const int MAX_GRANS = 8000;   // Max räckvidd för sensorn (ca 8 meter)
    bool har_skickat_notis = false; 

    ESP_LOGI(TAG, "Bevakning startad. Ignorerar allt närmare än %d mm.", MIN_GRANS);

    while (1) {
        int len = uart_read_bytes(UART_PORT, data, BUF_SIZE - 1, 50 / portTICK_PERIOD_MS);
        
        if (len > 0) {
            data[len] = '\0';
            char *text = (char *)data;

            char *range_ptr = strstr(text, "Range");
            if (range_ptr != NULL) {
                int avstand;
                if (sscanf(range_ptr, "Range %d", &avstand) == 1) {
                    
                    // Trigga bara om avståndet är STÖRRE än 1500 mm
                    if (avstand > MIN_GRANS && avstand < MAX_GRANS) {
                        if (!har_skickat_notis) {
                            ESP_LOGW(TAG, "🔔 NOTIS: Någon detekterad på avstånd! (%d mm)", avstand);
                            har_skickat_notis = true;
                        }
                    } 
                    // Återställ om det blir tomt eller om något kommer för nära igen
                    else {
                        if (har_skickat_notis) {
                            ESP_LOGI(TAG, "🌑 Info: Objektet lämnade zonen (eller kom för nära).");
                            har_skickat_notis = false;
                        }
                    }
                }
            }
        }
    }
}
// --- Huvudfunktion ---
extern "C" void app_main(void) {
    // 1. Konfigurera UART
    uart_config_t uart_config = {
        .baud_rate = 115200, // Fabriksstandard för LD2420
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    // 2. Installera drivrutinen
    ESP_ERROR_CHECK(uart_driver_install(UART_PORT, BUF_SIZE * 2, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_PORT, &uart_config));
    
    // 3. Sätt pinnar (TX, RX, RTS, CTS)
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT, SENSOR_TX_PIN, SENSOR_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    // 4. Starta tasken som sköter läsningen
    xTaskCreate(radar_task, "radar_task", 4096, NULL, 10, NULL);
}