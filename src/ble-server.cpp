#include <Arduino.h>
#include <NimBLEDevice.h>
#include "ble-server.h"


#define RTC_SERVICE_UUID "ccd187a1-f3be-464b-8a49-d989e9c104ce"
#define RTC_SYNC_CHARACTERISTIC_UUID "573c8962-c776-4016-a280-8daea13a01a8"

QueueHandle_t rtcQueue;

class RTCUpdateCallbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pRTCUpdate, NimBLEConnInfo& connInfo) override {
        std::string value = pRTCUpdate->getValue();

        if (value.length() > 0) {
            char buffer[19]; 
            strncpy(buffer, value.c_str(), sizeof(buffer) - 1);
            buffer[sizeof(buffer) - 1] = '\0'; 

            xQueueOverwrite(rtcQueue, buffer);
        }
    }
};

class ServerConnectionCallbacks : public NimBLEServerCallbacks {
public:

    void onConnect(
        NimBLEServer* pServer,
        NimBLEConnInfo& connInfo
    ) override {
        BLEServerManager::deviceConnected();
    }

    void onDisconnect(
        NimBLEServer* pServer,
        NimBLEConnInfo& connInfo,
        int reason
    ) override {
        BLEServerManager::deviceDisconnected();
    }
};

bool BLEServerManager::isConnected = false;
bool BLEServerManager::isInit = false;
NimBLEAdvertising * BLEServerManager::advertising = nullptr;
NimBLEServer * BLEServerManager::server = nullptr;

BLEServerManager::BLEServerManager()
{
}

void BLEServerManager::init() {
    rtcQueue = xQueueCreate(1, sizeof(char[19]));

    NimBLEDevice::init("NimBLE-ESP32-PIO");

    NimBLEServer *pServer = NimBLEDevice::createServer();
    NimBLEService *pRTC = pServer->createService(RTC_SERVICE_UUID);
    pServer->setCallbacks(new ServerConnectionCallbacks());
    server = pServer;

    NimBLECharacteristic *pRTCUpdate = pRTC->createCharacteristic(
        RTC_SYNC_CHARACTERISTIC_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE
    );

    pRTCUpdate->setValue("Unset");
    pRTCUpdate->setCallbacks(new RTCUpdateCallbacks());

    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(RTC_SERVICE_UUID);
    pAdvertising->start();
    advertising = pAdvertising;
    isInit = true;
}

void BLEServerManager::deviceConnected() {
    if (isInit) {
        isConnected = true;
    }
}

void BLEServerManager::deviceDisconnected() {
    if (isInit) {
        isConnected = false;
    }
}

bool BLEServerManager::getIsConnected() {
    if (isInit) {
        return isConnected;
    }
    else {
        return false;
    }
}

bool BLEServerManager::getIsAdvertising() {
    if (isInit) {
        if (advertising->isAdvertising())
        {
            return true;
        }
        else {
            return false;
        }
    }
    else {
        return false;
    }
}

void BLEServerManager::toggleBLE() {
    if (isInit) {
        switch (fetchBLEStates()) {
            case BLEEnum::CONNECTED:
                if (server != nullptr) {
                    for (uint16_t conn_id : server->getPeerDevices()) {
                        server->disconnect(conn_id); 
                    }
                }
                break;
            case BLEEnum::CONNECTING:
                advertising->stop();
                return;       
            case BLEEnum::DISCONNECTED:
                advertising->start();
                return;
        }
    }
}

BLEEnum BLEServerManager::fetchBLEStates() {
    if (isInit) {
        BLEEnum state;
        if (getIsConnected()) {
            state = BLEEnum::CONNECTED;
        }
        else if (getIsAdvertising()) {
            state = BLEEnum::CONNECTING;
        }
        else {
            state = BLEEnum::DISCONNECTED;
        }
        return state;
    }
    else {
        return BLEEnum::DISCONNECTED;
    }
}

BLEServerManager::~BLEServerManager()
{
}
