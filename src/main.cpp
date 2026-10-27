#include <Arduino.h>
#include <CST816S.h>
#include <config/CST816S_pin_config.h>
#include <lvgl.h>
#if LV_USE_TFT_ESPI
#include <TFT_eSPI.h>
#endif
#include <ui.h>
#include "ble-server.h"
#include "rtc-manager.h"

/*To use the built-in examples and demos of LVGL uncomment the includes below respectively.
 *You also need to copy `lvgl/examples` to `lvgl/src/examples`. Similarly for the demos `lvgl/demos` to `lvgl/src/demos`.
 *Note that the `lv_examples` library is for LVGL v7 and you shouldn't install it for this version (since LVGL v8)
 *as the examples and demos are now part of the main LVGL library. */

//#include <examples/lv_examples.h>
//#include <demos/lv_demos.h>

// Set Touch Driver
CST816S touch(TOUCH_SDA, TOUCH_SCL, TOUCH_RST, TOUCH_IRQ);	// sda, scl, rst, irq

/*Set to your screen resolution and rotation*/
#define TFT_HOR_RES   240
#define TFT_VER_RES   240
#define TFT_ROTATION  LV_DISPLAY_ROTATION_270

/*LVGL draw into this buffer, 1/10 screen size usually works well. The size is in bytes*/
#define DRAW_BUF_SIZE (TFT_HOR_RES * TFT_VER_RES / 10 * (LV_COLOR_DEPTH / 8))
uint32_t draw_buf[DRAW_BUF_SIZE / 4];

#if LV_USE_LOG != 0
void my_print( lv_log_level_t level, const char * buf )
{
    LV_UNUSED(level);
    Serial.println(buf);
    Serial.flush();
}
#endif

/* LVGL calls it when a rendered image needs to copied to the display*/
void my_disp_flush( lv_display_t *disp, const lv_area_t *area, uint8_t * px_map)
{
    /*Copy `px map` to the `area`*/

    /*For example ("my_..." functions needs to be implemented by you)
    uint32_t w = lv_area_get_width(area);
    uint32_t h = lv_area_get_height(area);

    my_set_window(area->x1, area->y1, w, h);
    my_draw_bitmaps(px_map, w * h);
     */

    /*Call it to tell LVGL you are ready*/
    lv_display_flush_ready(disp);
}

QueueHandle_t touchQueue;

struct TouchData {
    bool pressed;
    uint16_t x;
    uint16_t y;
};

/*Read the touchpad*/
void touchTask(void *pvParameters)
{
    touch.begin();
    TouchData data;
    while (1) {
        bool touched = touch.available();
        if(!touched) {
            data.pressed = false;
        } else {
            data.pressed = true;

            data.x = TFT_WIDTH - touch.data.x;
            data.y = TFT_HEIGHT - touch.data.y;

        }
            xQueueSend(touchQueue, &data, pdMS_TO_TICKS(10));
            vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void my_touchpad_read(lv_indev_t * indev, lv_indev_data_t * data) {
    TouchData receivedData;

    if (xQueueReceive(touchQueue, &receivedData, 0) == pdPASS) {
            if (receivedData.pressed) {
                data->state = LV_INDEV_STATE_PRESSED;
                data->point.x = receivedData.x;
                data->point.y = receivedData.y;
            } else {
                data->state = LV_INDEV_STATE_RELEASED;
            }
        } else {
            data->state = LV_INDEV_STATE_RELEASED;
    }
}

/*use Arduinos millis() as tick source*/
static uint32_t my_tick(void)
{
    return millis();
}

void setup()
{
    
    String LVGL_Arduino = "Hello Arduino! ";
    LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();

    Serial.begin( 9600 );
    Serial.println( LVGL_Arduino );
    
    lv_init();

    /*Set a tick source so that LVGL will know how much time elapsed. */
    lv_tick_set_cb(my_tick);

    /* register print function for debugging */
#if LV_USE_LOG != 0
    lv_log_register_print_cb( my_print );
#endif

    lv_display_t * disp;
#if LV_USE_TFT_ESPI
    /*TFT_eSPI can be enabled lv_conf.h to initialize the display in a simple way*/
    disp = lv_tft_espi_create(TFT_HOR_RES, TFT_VER_RES, draw_buf, sizeof(draw_buf));
    lv_display_set_rotation(disp, TFT_ROTATION);

#else
    /*Else create a display yourself*/
    disp = lv_display_create(TFT_HOR_RES, TFT_VER_RES);
    lv_display_set_flush_cb(disp, my_disp_flush);
    lv_display_set_buffers(disp, draw_buf, NULL, sizeof(draw_buf), LV_DISPLAY_RENDER_MODE_PARTIAL);
#endif

    touchQueue = xQueueCreate(1, sizeof(TouchData));

    xTaskCreatePinnedToCore(
        touchTask,
        "TouchTask",
        4096,
        NULL,
        2,
        NULL,
        0
    );

    /*Initialize the (dummy) input device driver*/
    lv_indev_t * indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER); /*Touchpad should have POINTER type*/
    lv_indev_set_read_cb(indev, my_touchpad_read);

    /* Create a simple label
     * ---------------------
     lv_obj_t *label = lv_label_create( lv_screen_active() );
     lv_label_set_text( label, "Hello Arduino, I'm LVGL!" );
     lv_obj_align( label, LV_ALIGN_CENTER, 0, 0 );

     * Try an example. See all the examples
     *  - Online: https://docs.lvgl.io/master/examples.html
     *  - Source codes: https://github.com/lvgl/lvgl/tree/master/examples
     * ----------------------------------------------------------------

     lv_example_btn_1();

     * Or try out a demo. Don't forget to enable the demos in lv_conf.h. E.g. LV_USE_DEMO_WIDGETS
     * -------------------------------------------------------------------------------------------

     lv_demo_widgets();
     */
    
    // lv_demo_music();

    ble_init();

    xTaskCreatePinnedToCore(
        rtc_recieve,
        "RTCTask",
        4096,
        NULL,
        2,
        NULL,
        0
    );

     ui_init();

    xTaskCreatePinnedToCore(
        rtc_print,
        "RTCPrint",
        2048,
        NULL,
        2,
        NULL,
        0
    );
}

void loop()
{
    lv_timer_handler(); /* let the GUI do its work */
    ui_tick();
    vTaskDelay(pdMS_TO_TICKS(5)); /* let this time pass */
}