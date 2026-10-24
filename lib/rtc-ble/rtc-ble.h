#pragma once

#include <Arduino.h>

// Initialize and start the BLE RTC server.
void beginRtcBleServer();

// Get the current system time in UTC.
// Returns "UNSET" if the clock has not been initialized.
String getRtcTime();
String getRtcDate();

// Set the system time in UTC.
// Format: YYYY-MM-DDTHH:MM:SS
// Returns true on success.
bool setRtcTime(const String& isoTime);
