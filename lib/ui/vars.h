#ifndef EEZ_LVGL_UI_VARS_H
#define EEZ_LVGL_UI_VARS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// enum declarations

// Flow global variables

enum FlowGlobalVariables {
    FLOW_GLOBAL_VARIABLE_DATE = 0,
    FLOW_GLOBAL_VARIABLE_BRIGHTNESS = 1,
    FLOW_GLOBAL_VARIABLE_BRIGHTNESS_STRING = 2,
    FLOW_GLOBAL_VARIABLE_BL_CONNECTING = 3,
    FLOW_GLOBAL_VARIABLE_BL_CONNECTED = 4,
    FLOW_GLOBAL_VARIABLE_BL_DISCONNECTED = 5,
    FLOW_GLOBAL_VARIABLE_BL_STRING = 6
};

// Native global variables

extern const char *get_var_date();
extern void set_var_date(const char *value);
extern int32_t get_var_brightness();
extern void set_var_brightness(int32_t value);
extern const char *get_var_brightness_string();
extern void set_var_brightness_string(const char *value);
extern bool get_var_bl_connecting();
extern void set_var_bl_connecting(bool value);
extern bool get_var_bl_connected();
extern void set_var_bl_connected(bool value);
extern bool get_var_bl_disconnected();
extern void set_var_bl_disconnected(bool value);
extern const char *get_var_bl_string();
extern void set_var_bl_string(const char *value);

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_VARS_H*/