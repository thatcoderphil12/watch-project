#pragma once

class DisplayManager
{
private:
    static uint8_t bl_pin;
    static uint8_t bl_value;
    static bool is_init;
public:
    DisplayManager();
    ~DisplayManager();
    static void init(uint8_t);
    static void shutdown();
    static uint8_t get_backlight();
    static void set_backlight(uint8_t);
};