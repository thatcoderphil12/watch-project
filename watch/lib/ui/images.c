#include "images.h"

const ext_img_desc_t images[7] = {
    { "Minute-Hand", &img_minute_hand },
    { "Seconds-Hand", &img_seconds_hand },
    { "Hours-Hand", &img_hours_hand },
    { "Hour-Markers", &img_hour_markers },
    { "Bluetooth", &img_bluetooth },
    { "Bluetooth-Connected", &img_bluetooth_connected },
    { "Bluetooth-Crossed", &img_bluetooth_crossed },
};