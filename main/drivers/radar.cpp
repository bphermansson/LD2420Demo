#include <drivers/radar.h>
#include <string.h>

// Skapa en unik tagg för denna driver
static const char *TAG = "LD2420_DRIVER";

void ld2420_driver_init() {
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    ESP_LOGI(TAG, "Initierar UART på pinnar TX:%d, RX:%d", SENSOR_TX_PIN, SENSOR_RX_PIN);
    
    ESP_ERROR_CHECK(uart_driver_install(UART_PORT, LD2420_BUF_SIZE * 2, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_PORT, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT, SENSOR_TX_PIN, SENSOR_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    
    ESP_LOGI(TAG, "Driver installerad korrekt.");
}

void ld2420_task(void *arg) {
    uint8_t data[LD2420_BUF_SIZE];
    
    // --- INSTÄLLNINGAR FÖR DIN ZON ---
    const int MIN_AVSTAND = 100; // 1 meter
    const int MAX_AVSTAND = 3000; // 3 meter
    bool i_zonen = false;

    ESP_LOGI(TAG, "Bevakning aktiv mellan %d mm och %d mm", MIN_AVSTAND, MAX_AVSTAND);

    while (1) {
        int len = uart_read_bytes(UART_PORT, data, LD2420_BUF_SIZE - 1, 100 / portTICK_PERIOD_MS);
        
        if (len > 0) {
            data[len] = '\0';
            char *range_ptr = strstr((char *)data, "Range");
            
            if (range_ptr) {
                int dist;
                if (sscanf(range_ptr, "Range %d", &dist) == 1) {
                    
                    // Kolla om avståndet är INOM ditt fönster (1-3 meter)
                    if (dist >= MIN_AVSTAND && dist <= MAX_AVSTAND) {
                        if (!i_zonen) {
                            ESP_LOGW(TAG, "🎯 OBJEKT ENTRÈ: %d mm (Inom 1-3m zonen)", dist);
                            i_zonen = true;
                            // Här kan du trigga en Matter-notis eller tända en LED
                        }
                    } 
                    // Om objektet försvinner utanför (antingen för nära eller för långt bort)
                    else {
                        if (i_zonen) {
                            ESP_LOGI(TAG, "🚫 OBJEKT UTTRÈ: %d mm (Utanför zonen)", dist);
                            i_zonen = false;
                        }
                    }
                }
            }
        }
    }
}