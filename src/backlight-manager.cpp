#include <Arduino.h>
#include "backlight-manager.h"

DisplayManager::DisplayManager()
{
}

void DisplayManager::init(uint8_t pin) {
    if (is_init != true) {
        bl_pin = pin;
        bl_value = 255;
        pinMode(bl_pin, OUTPUT);
        analogWrite(bl_pin, bl_value);
        is_init = true;
    }
}

uint8_t DisplayManager::get_backlight() {
    if (is_init) {
        return bl_value;
    }
    else {
        return 0;
    }
}

void DisplayManager::set_backlight(uint8_t value) {
    if (is_init) {
        analogWrite(bl_pin, value);
        bl_value = value;
    }
}

void DisplayManager::shutdown() {
    if (is_init) {
        analogWrite(bl_pin, 0);
        pinMode(bl_pin, INPUT);
        bl_value = 0;
        bl_pin = 0;
        is_init = false;
    }
}

DisplayManager::~DisplayManager()
{
}

uint8_t DisplayManager::bl_pin = 0;
uint8_t DisplayManager::bl_value = 0;
bool DisplayManager::is_init = false;