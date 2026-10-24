#include <Arduino.h>
#include <Wire.h>
#include <time.h>

#include <lvgl.h>
#include <TFT_eSPI.h>
#include <CST816S.h>
#include <ESP32Time.h>
#include <BLEDevice.h>

#include <esp_sleep.h>
#include <esp_pm.h>
#include <driver/gpio.h>
#include <driver/rtc_io.h>

#include <ui.h>
#include <rtc-ble.h>

// ============================================================
// HARDWARE: Waveshare ESP32-S3-Touch-LCD-1.28
// ============================================================

static constexpr int LCD_WIDTH = 240;
static constexpr int LCD_HEIGHT = 240;

static constexpr int BACKLIGHT_PIN = 2;
static constexpr int TOUCH_SDA_PIN = 6;
static constexpr int TOUCH_SCL_PIN = 7;
static constexpr int TOUCH_RESET_PIN = 13;
static constexpr gpio_num_t TOUCH_INT_PIN = GPIO_NUM_5;

// Screen turns off after 5 seconds.
// Deep sleep begins after 5 minutes of inactivity.
static constexpr uint32_t SCREEN_TIMEOUT_MS = 5000UL;
static constexpr uint32_t DEEP_SLEEP_TIMEOUT_MS = 300000UL;

// Ignore obviously invalid clock values before RTC time is synced.
static constexpr time_t MIN_VALID_TIME = 1704067200; // 2024-01-01 UTC

// ============================================================
// GLOBAL OBJECTS
// ============================================================

// Uses the existing PlatformIO TFT_eSPI configuration for the GC9A01A.
static TFT_eSPI tft;

// CST816S library by fbiego.
static CST816S touch(
    TOUCH_SDA_PIN,
    TOUCH_SCL_PIN,
    TOUCH_RESET_PIN,
    static_cast<int>(TOUCH_INT_PIN)
);

// System clock used by the watch face and RTC BLE code.
// If rtc-ble.h already DEFINES an object named "rtc", remove this
// definition and keep only one global definition in the project.
ESP32Time rtc;

// LVGL objects.
static lv_display_t *display = nullptr;
static lv_indev_t *touchInput = nullptr;
static lv_timer_t *clockUpdateTimer = nullptr;

// LVGL partial-render buffer: approximately one tenth of the screen.
alignas(4) static uint8_t drawBuffer[
    LCD_WIDTH * LCD_HEIGHT * sizeof(uint16_t) / 10
];

// ============================================================
// POWER MANAGEMENT STATE
// ============================================================

static uint32_t lastTouchMs = 0;
static bool screenIsOff = false;

static bool touchIsDown = false;
static bool ignoreTouchUntilRelease = false;
static bool wakeScreenRequested = false;

// ============================================================
// LVGL TICK SOURCE
// ============================================================

static uint32_t lvglTick()
{
    return static_cast<uint32_t>(millis());
}

// ============================================================
// CUSTOM TFT_eSPI FLUSH CALLBACK
// Uses the SAME TFT_eSPI object for drawing and sleep commands.
// ============================================================

static void displayFlush(
    lv_display_t *disp,
    const lv_area_t *area,
    uint8_t *pxMap)
{
    // While the display sleeps, skip SPI transfers. LVGL must still
    // receive flush-ready notifications so it does not stall.
    if (!screenIsOff) {
        const uint32_t width =
            static_cast<uint32_t>(area->x2 - area->x1 + 1);

        const uint32_t height =
            static_cast<uint32_t>(area->y2 - area->y1 + 1);

        tft.startWrite();
        tft.setAddrWindow(
            area->x1,
            area->y1,
            width,
            height
        );

        tft.pushColors(
            reinterpret_cast<uint16_t *>(pxMap),
            width * height,
            true
        );

        tft.endWrite();
    }

    lv_display_flush_ready(disp);
}

// ============================================================
// DISPLAY POWER CONTROL
// ============================================================

static void turnScreenOff()
{
    if (screenIsOff) {
        return;
    }

    // Prevent subsequent LVGL flush callbacks from sending pixels.
    screenIsOff = true;

    if (clockUpdateTimer != nullptr) {
        lv_timer_pause(clockUpdateTimer);
    }

    // Backlight off, then command the LCD controller to sleep.
    digitalWrite(BACKLIGHT_PIN, LOW);

    tft.writecommand(0x28);  // Display OFF
    delay(20);

    tft.writecommand(0x10);  // Sleep IN
    delay(120);

    Serial.println("Screen off; BLE remains initialized.");
}

static void turnScreenOn()
{
    if (!screenIsOff) {
        return;
    }

    // Wake the same LCD controller used by the LVGL display driver.
    tft.writecommand(0x11);  // Sleep OUT
    delay(120);

    tft.writecommand(0x29);  // Display ON
    delay(20);

    screenIsOff = false;

    if (clockUpdateTimer != nullptr) {
        lv_timer_resume(clockUpdateTimer);
    }

    // Redraw the active screen after waking the LCD.
    lv_obj_invalidate(lv_screen_active());

    digitalWrite(BACKLIGHT_PIN, HIGH);

    Serial.println("Screen awake.");
}

// ============================================================
// TOUCH INPUT
// ============================================================

static void readTouch(lv_indev_t *indev, lv_indev_data_t *data)
{
    if (touch.available()) {
        const uint32_t now = millis();

        // Any new controller event counts as activity.
        lastTouchMs = now;

        const bool released =
            (touch.data.event == 1) || (touch.data.points == 0);

        if (released) {
            touchIsDown = false;
            ignoreTouchUntilRelease = false;
        } else {
            touchIsDown = true;

            // If the display is off, use this tap only to wake it.
            if (screenIsOff) {
                wakeScreenRequested = true;
                ignoreTouchUntilRelease = true;
            }
        }
    }

    // Do not allow the tap that woke the screen to activate an LVGL
    // button or other control underneath it.
    if (ignoreTouchUntilRelease) {
        data->state = LV_INDEV_STATE_RELEASED;
    } else {
        data->state = touchIsDown
            ? LV_INDEV_STATE_PRESSED
            : LV_INDEV_STATE_RELEASED;
    }

    data->point.x = touch.data.x;
    data->point.y = touch.data.y;
}

// ============================================================
// CLOCK HANDS
//
// The names below assume your EEZ-generated "objects" structure
// contains hour_hand, minute_hand, and seconds_hand.
// Change those names if your ui.h generates different identifiers.
// ============================================================

static void updateClockHands(lv_timer_t *timer)
{
    LV_UNUSED(timer);

    if (screenIsOff || time(nullptr) < MIN_VALID_TIME) {
        return;
    }

    const int second = rtc.getSecond();
    const int minute = rtc.getMinute();
    const int hour = rtc.getHour();

    // LVGL image rotation uses tenths of a degree.
    const int32_t secondAngle = second * 60;
    const int32_t minuteAngle = minute * 60 + second;
    const int32_t hourAngle =
        (hour % 12) * 300 + minute * 5 + second / 12;

    if (objects.seconds_hand != nullptr) {
        lv_image_set_rotation(objects.seconds_hand, secondAngle);
    }

    if (objects.minutes_hand != nullptr) {
        lv_image_set_rotation(objects.minutes_hand, minuteAngle);
    }

    if (objects.hours_hand != nullptr) {
        lv_image_set_rotation(objects.hours_hand, hourAngle);
    }
}

// ============================================================
// AUTOMATIC LIGHT SLEEP
//
// BLE must remain initialized. Automatic light sleep requires
// ESP-IDF power management and FreeRTOS tickless idle support.
// ============================================================

static void configureAutomaticLightSleep()
{
    esp_err_t err = gpio_wakeup_enable(
        TOUCH_INT_PIN,
        GPIO_INTR_LOW_LEVEL
    );

    if (err == ESP_OK) {
        err = esp_sleep_enable_gpio_wakeup();
    }

    if (err != ESP_OK) {
        Serial.printf(
            "Touch light-sleep wake setup failed: %s\n",
            esp_err_to_name(err)
        );
    }

#if defined(CONFIG_PM_ENABLE) && CONFIG_PM_ENABLE

    esp_pm_config_t pmConfig = {};
    pmConfig.max_freq_mhz = 240;
    pmConfig.min_freq_mhz = 40;
    pmConfig.light_sleep_enable = true;

    err = esp_pm_configure(&pmConfig);

    if (err == ESP_OK) {
        Serial.println("Automatic light sleep configured.");
    } else {
        Serial.printf(
            "Automatic light sleep unavailable: %s\n",
            esp_err_to_name(err)
        );
    }

#else

    Serial.println(
        "Automatic light sleep is disabled in this build. "
        "Enable CONFIG_PM_ENABLE and FreeRTOS tickless idle "
        "in the ESP-IDF configuration."
    );

#endif
}

// ============================================================
// DEEP SLEEP
//
// BLE is stopped before deep sleep. A tap on the CST816S interrupt
// wakes the ESP32-S3, which restarts setup() and the UI.
// ============================================================

static void enterDeepSleep()
{
    Serial.println("Preparing for deep sleep.");

    // Configure wake-up before turning BLE off. If configuration
    // fails, keep the device operational.
    esp_err_t err = esp_sleep_enable_ext0_wakeup(
        TOUCH_INT_PIN,
        0  // Wake when the touch interrupt is LOW.
    );

    if (err != ESP_OK) {
        Serial.printf(
            "Deep-sleep wake setup failed: %s\n",
            esp_err_to_name(err)
        );
        return;
    }

    turnScreenOff();

    // Configure GPIO 5 as an RTC input for deep-sleep wake.
    rtc_gpio_init(TOUCH_INT_PIN);

    rtc_gpio_set_direction(
        TOUCH_INT_PIN,
        RTC_GPIO_MODE_INPUT_ONLY
    );

    rtc_gpio_pullup_en(TOUCH_INT_PIN);
    rtc_gpio_pulldown_dis(TOUCH_INT_PIN);

    // Standard ESP32 BLE Arduino library.
    // If rtc-ble.h uses NimBLE instead, use NimBLEDevice::deinit(true).
    BLEDevice::deinit(true);

    Serial.println("BLE stopped; entering deep sleep.");
    Serial.flush();

    esp_deep_sleep_start();

    // Execution does not continue beyond this point.
}

// ============================================================
// INACTIVITY SERVICE
// ============================================================

static void servicePowerManagement()
{
    const uint32_t now = millis();

    if (wakeScreenRequested) {
        wakeScreenRequested = false;
        turnScreenOn();
        return;
    }

    const uint32_t idleMs = now - lastTouchMs;

    // Deep sleep takes priority after five minutes.
    if (idleMs >= DEEP_SLEEP_TIMEOUT_MS) {
        enterDeepSleep();
        return;
    }

    // Turn off the display after five seconds.
    if (!screenIsOff && idleMs >= SCREEN_TIMEOUT_MS) {
        turnScreenOff();
    }
}

// ============================================================
// SETUP
// ============================================================

void setup()
{
    Serial.begin(9600);
    delay(100);

    Serial.println();
    Serial.println("Starting watch.");

    // GPIO 5 must be released from RTC mode when returning from
    // deep sleep before the touch library reinitializes it.
    if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT0) {
        rtc_gpio_deinit(TOUCH_INT_PIN);
    }

    pinMode(BACKLIGHT_PIN, OUTPUT);
    digitalWrite(BACKLIGHT_PIN, HIGH);

    // Eastern Time with daylight-saving transitions.
    // BLE timestamps must continue to represent UTC epoch time.
    setenv("TZ", "EST5EDT,M3.2.0/2,M11.1.0/2", 1);
    tzset();

    // --------------------------------------------------------
    // Initialize LVGL and the LCD.
    // --------------------------------------------------------

    lv_init();
    lv_tick_set_cb(lvglTick);

    tft.begin();
    tft.setRotation(135);

    display = lv_display_create(LCD_WIDTH, LCD_HEIGHT);

    if (display == nullptr) {
        Serial.println("ERROR: LVGL display creation failed.");
        while (true) {
            delay(1000);
        }
    }

    lv_display_set_flush_cb(display, displayFlush);

    lv_display_set_buffers(
        display,
        drawBuffer,
        nullptr,
        sizeof(drawBuffer),
        LV_DISPLAY_RENDER_MODE_PARTIAL
    );

    // --------------------------------------------------------
    // Initialize the CST816S touch controller.
    // --------------------------------------------------------

    pinMode(static_cast<int>(TOUCH_INT_PIN), INPUT_PULLUP);
    touch.begin(FALLING);

    touchInput = lv_indev_create();
    lv_indev_set_type(touchInput, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(touchInput, readTouch);

    // --------------------------------------------------------
    // Initialize the EEZ Studio-generated UI.
    // --------------------------------------------------------

    ui_init();

    clockUpdateTimer = lv_timer_create(
        updateClockHands,
        1000,
        nullptr
    );

    updateClockHands(nullptr);

    // --------------------------------------------------------
    // Initialize your existing BLE RTC server.
    // --------------------------------------------------------
    //
    // Start the RTC BLE service implemented in rtc-ble.cpp.
    // It exposes getRtcTime() and setRtcTime() via rtc-ble.h.
    // Incoming/outgoing ISO timestamps are UTC; the TZ setting above
    // controls local display time without changing the underlying epoch.
    beginRtcBleServer();

    // Initialize inactivity tracking only after setup is complete.
    lastTouchMs = millis();
    screenIsOff = false;

    configureAutomaticLightSleep();

    Serial.println("Setup complete.");
}

// ============================================================
// MAIN LOOP
// ============================================================

void loop()
{
    // Keep LVGL input processing alive even when the display is off.
    uint32_t delayMs = lv_timer_handler();

    servicePowerManagement();

    // Bound loop latency for touch handling.
    if (delayMs < 5) {
        delayMs = 5;
    }

    if (delayMs > 50) {
        delayMs = 50;
    }

    ui_tick();

    delay(delayMs);
}