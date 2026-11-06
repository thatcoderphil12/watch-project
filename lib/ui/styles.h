#ifndef EEZ_LVGL_UI_STYLES_H
#define EEZ_LVGL_UI_STYLES_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Style: 8-Bit (Text)
lv_style_t *get_style_8_bit__text__MAIN_DEFAULT();
void add_style_8_bit__text_(lv_obj_t *obj);
void remove_style_8_bit__text_(lv_obj_t *obj);

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_STYLES_H*/