#include <string>
#include <vars.h>
#include <Arduino.h>

static std::string date;

// Getter function called by the EEZ UI runtime
extern "C" const char *get_var_date() {
    String arduinoDate = "01/01/2026";
    date = arduinoDate.c_str();
    return date.c_str();
}