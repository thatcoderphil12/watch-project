#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

#include <sys/time.h>
#include <time.h>
#include <stdlib.h>

#include "rtc-ble.h"

namespace {

constexpr char DEVICE_NAME[] = "Waveshare-RTC";

constexpr char SERVICE_UUID[] =
    "f5ce7a24-8ef1-4adc-9ea4-eddb64c6e43d";

constexpr char TIME_UUID[] =
    "33bd8687-1a35-41d1-a4ac-15057c79c916";

constexpr time_t MIN_VALID_TIME = 1577836800; // 2020-01-01 UTC

// Parse a fixed number of decimal digits from a String.
bool parseDigits(
    const String& value,
    int position,
    int count,
    int& result
) {
    result = 0;

    for (int i = 0; i < count; ++i) {
        const char c = value[position + i];

        if (c < '0' || c > '9') {
            return false;
        }

        result = result * 10 + (c - '0');
    }

    return true;
}

bool isLeapYear(int year) {
    return (year % 4 == 0) &&
           ((year % 100 != 0) || (year % 400 == 0));
}

int daysInMonth(int year, int month) {
    constexpr int DAYS_PER_MONTH[] = {
        31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31
    };

    if (month < 1 || month > 12) {
        return 0;
    }

    if (month == 2 && isLeapYear(year)) {
        return 29;
    }

    return DAYS_PER_MONTH[month - 1];
}

bool isValidDateTime(
    int year,
    int month,
    int day,
    int hour,
    int minute,
    int second
) {
    if (year < 2020 || year > 2099) {
        return false;
    }

    if (month < 1 || month > 12) {
        return false;
    }

    if (day < 1 || day > daysInMonth(year, month)) {
        return false;
    }

    return hour >= 0 && hour <= 23 &&
           minute >= 0 && minute <= 59 &&
           second >= 0 && second <= 59;
}

} // namespace

String getRtcTime() {
    const time_t now = time(nullptr);

    if (now < MIN_VALID_TIME) {
        return "UNSET";
    }

    struct tm utc {};

    if (gmtime_r(&now, &utc) == nullptr) {
        return "UNSET";
    }

    char buffer[20];

    if (strftime(
            buffer,
            sizeof(buffer),
            "%Y-%m-%dT%H:%M:%S",
            &utc
        ) == 0) {
        return "UNSET";
    }

    return String(buffer);
}

static time_t utcToEpoch(
    int year, int month, int day,
    int hour, int minute, int second
) {
    int64_t days = 0;

    for (int y = 1970; y < year; ++y) {
        days += ((y % 4 == 0 && y % 100 != 0) ||
                 (y % 400 == 0)) ? 366 : 365;
    }

    for (int m = 1; m < month; ++m) {
        days += daysInMonth(year, m);
    }

    days += day - 1;

    return static_cast<time_t>(
        days * 86400LL +
        hour * 3600LL +
        minute * 60LL +
        second
    );
}

bool setRtcTime(const String& isoTime) {
    // Expected format: YYYY-MM-DDTHH:MM:SS
    if (isoTime.length() != 19 ||
        isoTime[4] != '-' ||
        isoTime[7] != '-' ||
        isoTime[10] != 'T' ||
        isoTime[13] != ':' ||
        isoTime[16] != ':') {
        return false;
    }

    int year, month, day;
    int hour, minute, second;

    if (!parseDigits(isoTime, 0, 4, year) ||
        !parseDigits(isoTime, 5, 2, month) ||
        !parseDigits(isoTime, 8, 2, day) ||
        !parseDigits(isoTime, 11, 2, hour) ||
        !parseDigits(isoTime, 14, 2, minute) ||
        !parseDigits(isoTime, 17, 2, second)) {
        return false;
    }

    if (!isValidDateTime(
            year, month, day,
            hour, minute, second)) {
        return false;
    }

    struct tm requested {};

    requested.tm_year = year - 1900;
    requested.tm_mon  = month - 1;
    requested.tm_mday = day;
    requested.tm_hour = hour;
    requested.tm_min  = minute;
    requested.tm_sec  = second;
    requested.tm_isdst = 0;

    // mktime() interprets the time as local time.
    // beginRtcBleServer() configures the local timezone as UTC.
    const time_t epoch = utcToEpoch(
    year, month, day,
    hour, minute, second
);

    if (epoch == static_cast<time_t>(-1)) {
        return false;
    }

    // Verify that the conversion preserved the requested UTC fields.
    struct tm verified {};

    if (gmtime_r(&epoch, &verified) == nullptr ||
        verified.tm_year != year - 1900 ||
        verified.tm_mon  != month - 1 ||
        verified.tm_mday != day ||
        verified.tm_hour != hour ||
        verified.tm_min  != minute ||
        verified.tm_sec  != second) {
        return false;
    }

    struct timeval tv {};
    tv.tv_sec = epoch;
    tv.tv_usec = 0;

    return settimeofday(&tv, nullptr) == 0;
}

class RtcBleCallbacks : public BLECharacteristicCallbacks {
public:
    void onRead(BLECharacteristic* characteristic) override {
        characteristic->setValue(getRtcTime());
    }

    void onWrite(BLECharacteristic* characteristic) override {
        const String value = characteristic->getValue();

        if (setRtcTime(value)) {
            const String currentTime = getRtcTime();

            Serial.print("RTC updated: ");
            Serial.println(currentTime);

            characteristic->setValue(currentTime);
        } else {
            Serial.println("RTC: rejected invalid date/time");

            // Restore the current value rather than retaining
            // the invalid string written by the BLE client.
            characteristic->setValue(getRtcTime());
        }
    }
};

void beginRtcBleServer() {
    BLEDevice::init(DEVICE_NAME);

    BLEServer* server = BLEDevice::createServer();
    BLEService* service = server->createService(SERVICE_UUID);

    BLECharacteristic* timeCharacteristic =
        service->createCharacteristic(
            TIME_UUID,
            BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE
        );

    timeCharacteristic->setCallbacks(new RtcBleCallbacks());
    timeCharacteristic->setValue(getRtcTime());

    service->start();

    BLEAdvertising* advertising = BLEDevice::getAdvertising();
    advertising->addServiceUUID(SERVICE_UUID);
    advertising->setScanResponse(true);
    advertising->start();

    Serial.println("RTC BLE server started");
    Serial.println(getRtcTime());
}