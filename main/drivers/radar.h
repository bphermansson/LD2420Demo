#pragma once

#include "driver/uart.h"
#include "esp_log.h"

// Inställningar för din driver
#define UART_PORT      UART_NUM_1
#define SENSOR_TX_PIN 9   // ESP32 TX (kopplas till sensorns RX)
#define SENSOR_RX_PIN 10  // ESP32 RX (kopplas till sensorns TX)
#define LD2420_BUF_SIZE       (1024)

// Funktionsprototyper
void ld2420_driver_init();
void ld2420_task(void *arg);