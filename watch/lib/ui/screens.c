#include <string.h>

#include "screens.h"
#include "images.h"
#include "fonts.h"
#include "actions.h"
#include "vars.h"
#include "styles.h"
#include "ui.h"

#include <string.h>

objects_t objects;

//
// Event handlers
//

lv_obj_t *tick_value_change_obj;

static void event_handler_cb_brightness_brightness_arc(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target_obj(e);
        if (tick_value_change_obj != ta) {
            int32_t value = lv_arc_get_value(ta);
            set_var_brightness(value);
        }
    }
}

//
// Screens
//

void create_screen_main() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.main = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 240, 240);
    {
        lv_obj_t *parent_obj = obj;
        {
            lv_obj_t *obj = lv_image_create(parent_obj);
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_image_set_src(obj, &img_hour_markers);
        }
        {
            // Date
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.date = obj;
            lv_obj_set_pos(obj, 61, 79);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            add_style_8_bit__text_(obj);
            lv_label_set_text(obj, "");
        }
        {
            // Seconds-Hand
            lv_obj_t *obj = lv_image_create(parent_obj);
            objects.seconds_hand = obj;
            lv_obj_set_pos(obj, 120, 9);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_image_set_src(obj, &img_seconds_hand);
            lv_image_set_pivot(obj, 0, 111);
        }
        {
            // Minutes-Hand
            lv_obj_t *obj = lv_image_create(parent_obj);
            objects.minutes_hand = obj;
            lv_obj_set_pos(obj, 119, 17);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_image_set_src(obj, &img_minute_hand);
            lv_image_set_pivot(obj, 0, 103);
        }
        {
            // Hours-Hand
            lv_obj_t *obj = lv_image_create(parent_obj);
            objects.hours_hand = obj;
            lv_obj_set_pos(obj, 119, 37);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_image_set_src(obj, &img_hours_hand);
            lv_image_set_pivot(obj, 0, 83);
            lv_image_set_rotation(obj, 9);
        }
    }
    
    tick_screen_main();
}

void tick_screen_main() {
    {
        const char *new_val = get_var_date();
        const char *cur_val = lv_label_get_text(objects.date);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.date;
            lv_label_set_text(objects.date, new_val);
            tick_value_change_obj = NULL;
        }
    }
}

void create_screen_brightness() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.brightness = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 240, 240);
    lv_obj_set_style_text_color(obj, lv_color_hex(0x1565c0), LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // Brightness-Arc
            lv_obj_t *obj = lv_arc_create(parent_obj);
            objects.brightness_arc = obj;
            lv_obj_set_pos(obj, 20, 20);
            lv_obj_set_size(obj, 200, 200);
            lv_arc_set_range(obj, 0, 255);
            lv_obj_add_event_cb(obj, event_handler_cb_brightness_brightness_arc, LV_EVENT_ALL, 0);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_ADV_HITTEST);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_GESTURE_BUBBLE);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x1565c0), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // brightness_text
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.brightness_text = obj;
            lv_obj_set_pos(obj, 102, 115);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            add_style_8_bit__text_(obj);
            lv_label_set_text(obj, "");
        }
    }
    
    tick_screen_brightness();
}

void tick_screen_brightness() {
    {
        int32_t new_val = get_var_brightness();
        int32_t cur_val = lv_arc_get_value(objects.brightness_arc);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.brightness_arc;
            lv_arc_set_value(objects.brightness_arc, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_brightness_string();
        const char *cur_val = lv_label_get_text(objects.brightness_text);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.brightness_text;
            lv_label_set_text(objects.brightness_text, new_val);
            tick_value_change_obj = NULL;
        }
    }
}

void create_screen_bluetooth() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.bluetooth = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 240, 240);
    {
        lv_obj_t *parent_obj = obj;
        {
            // bluetooth-image-connecting
            lv_obj_t *obj = lv_image_create(parent_obj);
            objects.bluetooth_image_connecting = obj;
            lv_obj_set_pos(obj, 56, 56);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_image_set_src(obj, &img_bluetooth);
        }
        {
            // bluetooth-image-connected
            lv_obj_t *obj = lv_image_create(parent_obj);
            objects.bluetooth_image_connected = obj;
            lv_obj_set_pos(obj, 56, 56);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_image_set_src(obj, &img_bluetooth_connected);
        }
        {
            // bluetooth-image-disconnected
            lv_obj_t *obj = lv_image_create(parent_obj);
            objects.bluetooth_image_disconnected = obj;
            lv_obj_set_pos(obj, 56, 56);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_image_set_src(obj, &img_bluetooth_crossed);
        }
        {
            // bluetooth_status
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.bluetooth_status = obj;
            lv_obj_set_pos(obj, 45, 184);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            add_style_8_bit__text_(obj);
            lv_obj_set_style_width(obj, 150, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // ble_toggle_button
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.ble_toggle_button = obj;
            lv_obj_set_pos(obj, 88, 72);
            lv_obj_set_size(obj, 64, 96);
            lv_obj_add_event_cb(obj, action_ble_toggle, LV_EVENT_LONG_PRESSED, (void *)0);
            lv_obj_set_style_width(obj, 64, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_height(obj, 96, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
    
    tick_screen_bluetooth();
}

void tick_screen_bluetooth() {
    {
        bool new_val = get_var_bl_connecting();
        bool cur_val = lv_obj_has_flag(objects.bluetooth_image_connecting, LV_OBJ_FLAG_HIDDEN);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.bluetooth_image_connecting;
            if (new_val) {
                lv_obj_add_flag(objects.bluetooth_image_connecting, LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_remove_flag(objects.bluetooth_image_connecting, LV_OBJ_FLAG_HIDDEN);
            }
            tick_value_change_obj = NULL;
        }
    }
    {
        bool new_val = get_var_bl_connected();
        bool cur_val = lv_obj_has_flag(objects.bluetooth_image_connected, LV_OBJ_FLAG_HIDDEN);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.bluetooth_image_connected;
            if (new_val) {
                lv_obj_add_flag(objects.bluetooth_image_connected, LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_remove_flag(objects.bluetooth_image_connected, LV_OBJ_FLAG_HIDDEN);
            }
            tick_value_change_obj = NULL;
        }
    }
    {
        bool new_val = get_var_bl_disconnected();
        bool cur_val = lv_obj_has_flag(objects.bluetooth_image_disconnected, LV_OBJ_FLAG_HIDDEN);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.bluetooth_image_disconnected;
            if (new_val) {
                lv_obj_add_flag(objects.bluetooth_image_disconnected, LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_remove_flag(objects.bluetooth_image_disconnected, LV_OBJ_FLAG_HIDDEN);
            }
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_bl_string();
        const char *cur_val = lv_label_get_text(objects.bluetooth_status);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.bluetooth_status;
            lv_label_set_text(objects.bluetooth_status, new_val);
            tick_value_change_obj = NULL;
        }
    }
}

typedef void (*tick_screen_func_t)();
tick_screen_func_t tick_screen_funcs[] = {
    tick_screen_main,
    tick_screen_brightness,
    tick_screen_bluetooth,
};
void tick_screen(int screen_index) {
    if (screen_index >= 0 && screen_index < 3) {
        tick_screen_funcs[screen_index]();
    }
}
void tick_screen_by_id(enum ScreensEnum screenId) {
    tick_screen(screenId - 1);
}

//
// Fonts
//

ext_font_desc_t fonts[] = {
    { "Press_Start_2P", &ui_font_press_start_2_p },
#if LV_FONT_MONTSERRAT_8
    { "MONTSERRAT_8", &lv_font_montserrat_8 },
#endif
#if LV_FONT_MONTSERRAT_10
    { "MONTSERRAT_10", &lv_font_montserrat_10 },
#endif
#if LV_FONT_MONTSERRAT_12
    { "MONTSERRAT_12", &lv_font_montserrat_12 },
#endif
#if LV_FONT_MONTSERRAT_14
    { "MONTSERRAT_14", &lv_font_montserrat_14 },
#endif
#if LV_FONT_MONTSERRAT_16
    { "MONTSERRAT_16", &lv_font_montserrat_16 },
#endif
#if LV_FONT_MONTSERRAT_18
    { "MONTSERRAT_18", &lv_font_montserrat_18 },
#endif
#if LV_FONT_MONTSERRAT_20
    { "MONTSERRAT_20", &lv_font_montserrat_20 },
#endif
#if LV_FONT_MONTSERRAT_22
    { "MONTSERRAT_22", &lv_font_montserrat_22 },
#endif
#if LV_FONT_MONTSERRAT_24
    { "MONTSERRAT_24", &lv_font_montserrat_24 },
#endif
#if LV_FONT_MONTSERRAT_26
    { "MONTSERRAT_26", &lv_font_montserrat_26 },
#endif
#if LV_FONT_MONTSERRAT_28
    { "MONTSERRAT_28", &lv_font_montserrat_28 },
#endif
#if LV_FONT_MONTSERRAT_30
    { "MONTSERRAT_30", &lv_font_montserrat_30 },
#endif
#if LV_FONT_MONTSERRAT_32
    { "MONTSERRAT_32", &lv_font_montserrat_32 },
#endif
#if LV_FONT_MONTSERRAT_34
    { "MONTSERRAT_34", &lv_font_montserrat_34 },
#endif
#if LV_FONT_MONTSERRAT_36
    { "MONTSERRAT_36", &lv_font_montserrat_36 },
#endif
#if LV_FONT_MONTSERRAT_38
    { "MONTSERRAT_38", &lv_font_montserrat_38 },
#endif
#if LV_FONT_MONTSERRAT_40
    { "MONTSERRAT_40", &lv_font_montserrat_40 },
#endif
#if LV_FONT_MONTSERRAT_42
    { "MONTSERRAT_42", &lv_font_montserrat_42 },
#endif
#if LV_FONT_MONTSERRAT_44
    { "MONTSERRAT_44", &lv_font_montserrat_44 },
#endif
#if LV_FONT_MONTSERRAT_46
    { "MONTSERRAT_46", &lv_font_montserrat_46 },
#endif
#if LV_FONT_MONTSERRAT_48
    { "MONTSERRAT_48", &lv_font_montserrat_48 },
#endif
};

//
// Color themes
//

uint32_t active_theme_index = 0;
void change_color_theme(uint32_t theme_index) {
    active_theme_index = theme_index;
    
    lv_style_set_text_color(get_style_8_bit__text__MAIN_DEFAULT(), lv_color_hex(theme_colors[theme_index][0]));
    lv_obj_invalidate(objects.main);
    lv_obj_invalidate(objects.brightness);
    lv_obj_invalidate(objects.bluetooth);
}
uint32_t theme_colors[1][1] = {
    { 0xff1565c0 },
};

//
//
//

void create_screens() {

// Set default LVGL theme
    lv_display_t *dispp = lv_display_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), true, LV_FONT_DEFAULT);
    lv_display_set_theme(dispp, theme);
    
    // Initialize screens
    // Create screens
    create_screen_main();
    create_screen_brightness();
    create_screen_bluetooth();
}