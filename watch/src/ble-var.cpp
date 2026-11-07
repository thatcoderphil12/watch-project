#include "ble-server.h"
#include <vars.h>
#include "actions.h"
#include <string>

// bl_connecting var

extern "C" bool get_var_bl_connecting() {
    if (BLEServerManager::fetchBLEStates() == BLEEnum::CONNECTING) {
        return false;
    }
    else {
        return true;
    }
}

// bl_connected var

extern "C" bool get_var_bl_connected() {
    if (BLEServerManager::fetchBLEStates() == BLEEnum::CONNECTED) {
        return false;
    }
    else {
        return true;
    }
}

// bl_disconnected var

extern "C" bool get_var_bl_disconnected() {
    if (BLEServerManager::fetchBLEStates() == BLEEnum::DISCONNECTED) {
        return false;
    }
    else {
        return true;
    }
}

std::string bl_string;

extern "C" const char *get_var_bl_string() {
    switch (BLEServerManager::fetchBLEStates()) {
        case BLEEnum::CONNECTED:
            bl_string = "Connected";
            break;
        case BLEEnum::CONNECTING:
            bl_string = "Connecting";
            break;
        case BLEEnum::DISCONNECTED:
            bl_string = "Disconnected";
            break;
    }
    return bl_string.c_str();
}

// Button event handling

extern "C" void action_ble_toggle(lv_event_t *e) {
    BLEServerManager::toggleBLE();
}
