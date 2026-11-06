#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Screens

enum ScreensEnum {
    _SCREEN_ID_FIRST = 1,
    SCREEN_ID_MAIN = 1,
    SCREEN_ID_BRIGHTNESS = 2,
    SCREEN_ID_BLUETOOTH = 3,
    _SCREEN_ID_LAST = 3
};

typedef struct _objects_t {
    lv_obj_t *main;
    lv_obj_t *brightness;
    lv_obj_t *bluetooth;
    lv_obj_t *date;
    lv_obj_t *seconds_hand;
    lv_obj_t *minutes_hand;
    lv_obj_t *hours_hand;
    lv_obj_t *brightness_arc;
    lv_obj_t *brightness_text;
    lv_obj_t *bluetooth_image_connecting;
    lv_obj_t *bluetooth_image_connected;
    lv_obj_t *bluetooth_image_disconnected;
    lv_obj_t *bluetooth_status;
    lv_obj_t *ble_toggle_button;
} objects_t;

extern objects_t objects;

void create_screen_main();
void tick_screen_main();

void create_screen_brightness();
void tick_screen_brightness();

void create_screen_bluetooth();
void tick_screen_bluetooth();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

// Color themes

enum Themes {
    THEME_ID_8_BIT,
};
enum Colors {
    COLOR_ID_MAIN,
};
void change_color_theme(uint32_t themeIndex);
extern uint32_t theme_colors[1][1];
extern uint32_t active_theme_index;

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/