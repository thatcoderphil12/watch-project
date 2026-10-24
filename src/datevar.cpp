#include <string>
#include <vars.h>
#include <Arduino.h>
#include <rtc-ble.h>

// Native string storage
static std::string date;

// Getter function called by the EEZ UI runtime
extern "C" const char *get_var_date() {
    String arduinoDate = getRtcDate();
    date = arduinoDate.c_str();
    return date.c_str();
}