#include <Arduino.h>
#include "ble-server.h"
#include "rtc-manager.h"

#include <ESP32Time.h>

// Change -18,000 based off of your timezone (Currently configured for US-East)
ESP32Time rtc(-18000);

void rtc_recieve(void *pvParameters) {
    char receivedBuffer[19];
    while(true) {
        if (xQueueReceive(rtcQueue, receivedBuffer, portMAX_DELAY)) {
            unsigned long epochTime = strtoul(receivedBuffer, NULL, 10);
            rtc.setTime(epochTime);
        }
    }
};

time_struct get_rtc_time() {
    time_struct result;
    result.hour = rtc.getHour();
    result.minute = rtc.getMinute();
    result.second = rtc.getSecond();
    return result;
};

String get_rtc_date() {
    String result;
    result = rtc.getTime("%m/%d/%Y");
    return result;
};