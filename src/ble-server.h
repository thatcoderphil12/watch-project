#pragma once

#include <NimBLEDevice.h>

extern QueueHandle_t rtcQueue; 

enum class BLEEnum {
    CONNECTED,
    CONNECTING,
    DISCONNECTED
};

class BLEServerManager
{
private:
    static bool isInit;
    static bool isConnected;
    static NimBLEAdvertising *advertising;
    static NimBLEServer *server;
public:
    BLEServerManager();
    static void init();
    static void deviceConnected();
    static void deviceDisconnected();
    static bool getIsConnected();
    static bool getIsAdvertising();
    static void toggleBLE();
    static BLEEnum fetchBLEStates();
    ~BLEServerManager();
};