#pragma once

void rtc_recieve(void *pvParameters);

typedef struct {
    int_fast8_t second;
    int_fast8_t minute;
    int_fast8_t hour;
} time_struct;

String get_rtc_date();
time_struct get_rtc_time();