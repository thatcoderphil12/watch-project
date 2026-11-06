#include "styles.h"
#include "images.h"
#include "fonts.h"

#include "ui.h"
#include "screens.h"

//
// Style: 8-Bit (Text)
//

void init_style_8_bit__text__MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_text_color(style, lv_color_hex(theme_colors[active_theme_index][0]));
    lv_style_set_text_font(style, &ui_font_press_start_2_p);
    lv_style_set_text_opa(style, 255);
};

lv_style_t *get_style_8_bit__text__MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_malloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_8_bit__text__MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_8_bit__text_(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_8_bit__text__MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_8_bit__text_(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_8_bit__text__MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
//
//

void add_style(lv_obj_t *obj, int32_t styleIndex) {
    typedef void (*AddStyleFunc)(lv_obj_t *obj);
    static const AddStyleFunc add_style_funcs[] = {
        add_style_8_bit__text_,
    };
    add_style_funcs[styleIndex](obj);
}

void remove_style(lv_obj_t *obj, int32_t styleIndex) {
    typedef void (*RemoveStyleFunc)(lv_obj_t *obj);
    static const RemoveStyleFunc remove_style_funcs[] = {
        remove_style_8_bit__text_,
    };
    remove_style_funcs[styleIndex](obj);
}