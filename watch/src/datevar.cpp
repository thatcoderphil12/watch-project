#include <string>
#include <vars.h>
#include <Arduino.h>
#include "rtc-manager.h"

static std::string date;

// Getter function called by the EEZ UI runtime
extern "C" const char *get_var_date() {
    String arduinoDate = get_rtc_date();
    date = arduinoDate.c_str();
    return date.c_str();
}