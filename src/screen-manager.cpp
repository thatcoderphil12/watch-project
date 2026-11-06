#include <lvgl.h>
#include <ui.h>
#include "screen-manager.h"

void screen_gesture_cb_main(lv_event_t * e)
{
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());

    switch (dir) {
        case LV_DIR_TOP:
            loadScreen(SCREEN_ID_BRIGHTNESS);
            break;
    }
}

void screen_gesture_cb_brightness(lv_event_t * e)
{
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());

    switch (dir) {
        case LV_DIR_TOP:
            loadScreen(SCREEN_ID_BLUETOOTH);
            break;

        case LV_DIR_BOTTOM:
            loadScreen(SCREEN_ID_MAIN);
            break;
    }
}

void screen_gesture_cb_bluetooth(lv_event_t * e)
{
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());

    switch (dir) {
        case LV_DIR_BOTTOM:
            loadScreen(SCREEN_ID_BRIGHTNESS);
            break;
    }
}