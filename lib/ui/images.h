#ifndef EEZ_LVGL_UI_IMAGES_H
#define EEZ_LVGL_UI_IMAGES_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const lv_img_dsc_t img_minute_hand;
extern const lv_img_dsc_t img_seconds_hand;
extern const lv_img_dsc_t img_hours_hand;
extern const lv_img_dsc_t img_hour_markers;
extern const lv_img_dsc_t img_bluetooth;
extern const lv_img_dsc_t img_bluetooth_connected;
extern const lv_img_dsc_t img_bluetooth_crossed;

#ifndef EXT_IMG_DESC_T
#define EXT_IMG_DESC_T
typedef struct _ext_img_desc_t {
    const char *name;
    const lv_img_dsc_t *img_dsc;
} ext_img_desc_t;
#endif

extern const ext_img_desc_t images[7];

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_IMAGES_H*/