#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <vector>
#include "config.h"

enum WirelessProtocol {
    PROTO_WIFI = 0,
    PROTO_BLE
};

enum ScannerState {
    SCAN_STATE_IDLE = 0,
    SCAN_STATE_SCANNING,
    SCAN_STATE_TRACKING
};

struct TargetDevice {
    WirelessProtocol proto;
    String name;          // SSID or BLE Name
    String address;       // BSSID or MAC ("AA:BB:CC:DD:EE:FF")
    int rssi;             // dBm
    uint8_t channel;      // Wi-Fi channel (1-14)
    String extra;         // "WPA2", "Open", "BLE Adv", etc.
    unsigned long lastSeenMs;
};

class Scanner : public BLEAdvertisedDeviceCallbacks {
public:
    Scanner();

    void begin();
    void update();

    // Protocol Management
    void setProtocol(WirelessProtocol proto);
    WirelessProtocol getProtocol() const { return _currentProto; }
    const char* getProtocolName() const { return (_currentProto == PROTO_WIFI) ? "Wi-Fi 2.4GHz" : "Bluetooth LE"; }

    // Scan Operations
    void startScan();
    void stopScan();
    bool isScanning() const { return _state == SCAN_STATE_SCANNING; }

    // Target Selection & Foxhunting
    void selectTarget(int index);
    void selectTargetByAddress(const String &address);
    void stopTracking();
    bool isTracking() const { return _state == SCAN_STATE_TRACKING; }

    // Target List Access
    int getTargetCount() const;
    bool getTarget(int index, TargetDevice &out) const;

    // Foxhunting Live Metrics
    const TargetDevice& getTrackedTarget() const { return _trackedTarget; }
    int getCurrentRssi() const { return _currentRssi; }
    int getPeakRssi() const { return _peakRssi; }
    int getMinRssi() const { return _minRssi; }
    float getSmoothedRssi() const { return _smoothedRssi; }
    float getSignalPercent() const { return _signalPercent; }
    float getPacketsPerSec() const { return _packetsPerSec; }
    unsigned long getLastPacketAgeMs() const { return millis() - _lastPacketMs; }

    // 2D History Buffer for Rolling Graph
    static const int HISTORY_LEN = 60;
    int8_t getHistory(int index) const; // 0 = oldest, HISTORY_LEN-1 = newest

    // Calibration
    void adjustMinRssi(int delta);
    void adjustMaxRssi(int delta);
    int getMinRssiThreshold() const { return _minRssiThreshold; }
    int getMaxRssiThreshold() const { return _maxRssiThreshold; }
    void resetMetrics();

    // BLE Callback override
    virtual void onResult(BLEAdvertisedDevice advertisedDevice) override;

private:
    WirelessProtocol _currentProto;
    ScannerState _state;

    std::vector<TargetDevice> _targets;
    TargetDevice _trackedTarget;
    portMUX_TYPE _mux;

    // Metrics
    int _currentRssi;
    int _peakRssi;
    int _minRssi;
    float _smoothedRssi;
    float _signalPercent;
    unsigned long _lastPacketMs;

    uint32_t _packetCount;
    float _packetsPerSec;
    unsigned long _rateWindowMs;
    uint32_t _windowPackets;

    // Rolling Graph History
    int8_t _rssiHistory[HISTORY_LEN];
    unsigned long _lastHistoryPushMs;

    // Thresholds
    int _minRssiThreshold;
    int _maxRssiThreshold;

    // Hardware handles
    BLEScan *_pBLEScan;
    bool _bleInitialized;
    bool _wifiInitialized;

    // Non-blocking scan timers
    unsigned long _scanStartMs;
    unsigned long _lastWifiScanTriggerMs;

    void initWiFi();
    void deinitWiFi();

    void initBLE();
    void deinitBLE();

    void updateWiFiScan();
    void updateWiFiTracking();

    void updateBLEScan();
    void updateBLETracking();

    void recordRssiSample(int rssi);
    float calculatePercent(int rssi);
};

extern Scanner RadioScanner;
