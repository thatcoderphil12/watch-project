#include <vars.h>
#include <string>
#include "backlight-manager.h"

// EEZ Studio logic is here
std::string brightness_string;
uint8_t brightness;

extern "C" int32_t get_var_brightness() { 
    return static_cast<int32_t>(DisplayManager::get_backlight());
}

extern "C" void set_var_brightness(int32_t value) {
    DisplayManager::set_backlight(static_cast<uint8_t>(value));
}

extern "C" const char *get_var_brightness_string() {
    brightness = DisplayManager::get_backlight();
    brightness /= 2.55;
    brightness_string = std::to_string(brightness);
    brightness_string += '%';
    return brightness_string.c_str();
}
