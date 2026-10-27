#include <Arduino.h>
#include "ble-server.h"
#include "rtc-manager.h"

#include <ESP32Time.h>

ESP32Time rtc(-14400);

void rtc_recieve(void *pvParameters) {
    char receivedBuffer[19];
    while(true) {
        if (xQueueReceive(rtcQueue, receivedBuffer, portMAX_DELAY)) {
            unsigned long epochTime = strtoul(receivedBuffer, NULL, 10);
            rtc.setTime(epochTime);
        }
    }
}

void rtc_print(void *pvParameters) {
    while (true)
    {
        Serial.println(rtc.getTime("%A %B %d %Y %H:%M:%S"));
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}