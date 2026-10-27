#include <Arduino.h>
#include <NimBLEDevice.h>
#include "ble-server.h"


#define SERVICE_UUID "ccd187a1-f3be-464b-8a49-d989e9c104ce"
#define CHARACTERISTIC_UUID "573c8962-c776-4016-a280-8daea13a01a8"

QueueHandle_t rtcQueue;

class RTCUpdateCallbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pRTCUpdate, NimBLEConnInfo& connInfo) override {
        std::string value = pRTCUpdate->getValue();

        if (value.length() > 0) {
            char buffer[19]; 
            strncpy(buffer, value.c_str(), sizeof(buffer) - 1);
            buffer[sizeof(buffer) - 1] = '\0'; 

            xQueueSend(rtcQueue, buffer, 0);
        }
    }
};

void ble_init() {

    rtcQueue = xQueueCreate(1, sizeof(char[19]));

    NimBLEDevice::init("NimBLE-ESP32-PIO");

    NimBLEServer *pServer = NimBLEDevice::createServer();
    NimBLEService *pRTC = pServer->createService(SERVICE_UUID);

    NimBLECharacteristic *pRTCUpdate = pRTC->createCharacteristic(
        CHARACTERISTIC_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE
    );

    pRTCUpdate->setValue("Unset");
    pRTCUpdate->setCallbacks(new RTCUpdateCallbacks());

    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->start();
}

