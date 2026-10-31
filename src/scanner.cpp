#include "scanner.h"
#include "led_meter.h"
#include <algorithm>

Scanner RadioScanner;

static String authModeToString(wifi_auth_mode_t auth) {
    switch (auth) {
        case WIFI_AUTH_OPEN: return "Open";
        case WIFI_AUTH_WEP: return "WEP";
        case WIFI_AUTH_WPA_PSK: return "WPA";
        case WIFI_AUTH_WPA2_PSK: return "WPA2";
        case WIFI_AUTH_WPA_WPA2_PSK: return "WPA/WPA2";
        case WIFI_AUTH_WPA2_ENTERPRISE: return "WPA2-Ent";
        case WIFI_AUTH_WPA3_PSK: return "WPA3";
        case WIFI_AUTH_WPA2_WPA3_PSK: return "WPA2/WPA3";
        default: return "Enc";
    }
}

static String extractBleName(BLEAdvertisedDevice &dev) {
    if (dev.haveName() && dev.getName().length() > 0) {
        return dev.getName().c_str();
    }
    uint8_t *payload = dev.getPayload();
    size_t len = dev.getPayloadLength();
    if (!payload || len == 0) return "";

    size_t idx = 0;
    while (idx < len) {
        uint8_t fieldLen = payload[idx];
        if (fieldLen == 0 || idx + 1 + fieldLen > len) break;
        uint8_t type = payload[idx + 1];
        if (type == 0x08 || type == 0x09) { // Shortened or Complete Name
            char nameBuf[32];
            size_t nameLen = min((size_t)(fieldLen - 1), sizeof(nameBuf) - 1);
            memcpy(nameBuf, &payload[idx + 2], nameLen);
            nameBuf[nameLen] = '\0';
            return String(nameBuf);
        }
        idx += 1 + fieldLen;
    }
    return "";
}

Scanner::Scanner()
    : _currentProto(PROTO_WIFI),
      _state(SCAN_STATE_IDLE),
      _currentRssi(-100),
      _peakRssi(-100),
      _minRssi(0),
      _smoothedRssi(-100.0f),
      _signalPercent(0.0f),
      _lastPacketMs(0),
      _packetCount(0),
      _packetsPerSec(0.0f),
      _rateWindowMs(0),
      _windowPackets(0),
      _lastHistoryPushMs(0),
      _minRssiThreshold(FOXHUNT_MIN_RSSI),
      _maxRssiThreshold(FOXHUNT_MAX_RSSI),
      _pBLEScan(nullptr),
      _bleInitialized(false),
      _wifiInitialized(false),
      _scanStartMs(0),
      _lastWifiScanTriggerMs(0)
{
    _mux = portMUX_INITIALIZER_UNLOCKED;
    for (int i = 0; i < HISTORY_LEN; i++) {
        _rssiHistory[i] = _minRssiThreshold;
    }
}

void Scanner::begin() {
    initWiFi();
    startScan();
}

void Scanner::initWiFi() {
    if (_bleInitialized && _pBLEScan) {
        _pBLEScan->stop();
    }
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    _wifiInitialized = true;
    _currentProto = PROTO_WIFI;
}

void Scanner::deinitWiFi() {
    if (!_wifiInitialized) return;
    WiFi.scanDelete();
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    _wifiInitialized = false;
}

void Scanner::initBLE() {
    if (_wifiInitialized) deinitWiFi();
    if (!_bleInitialized) {
        BLEDevice::init("SaintConFox");
        _pBLEScan = BLEDevice::getScan();
        if (_pBLEScan) {
            _pBLEScan->setAdvertisedDeviceCallbacks(this, true, false); // shouldParse = false!
            _pBLEScan->setActiveScan(false);
            _pBLEScan->setInterval(100);
            _pBLEScan->setWindow(99);
        }
        _bleInitialized = true;
    }
    _currentProto = PROTO_BLE;
}

void Scanner::deinitBLE() {
    if (_pBLEScan) {
        _pBLEScan->stop();
        _pBLEScan->clearResults();
    }
}

void Scanner::setProtocol(WirelessProtocol proto) {
    if (_currentProto == proto && (_wifiInitialized || _bleInitialized)) return;

    stopTracking();
    stopScan();

    if (proto == PROTO_WIFI) {
        if (_bleInitialized && _pBLEScan) {
            _pBLEScan->stop();
        }
        initWiFi();
    } else {
        deinitWiFi();
        initBLE();
    }
    _currentProto = proto;
    startScan();
}

void Scanner::startScan() {
    portENTER_CRITICAL(&_mux);
    _targets.clear();
    portEXIT_CRITICAL(&_mux);

    _state = SCAN_STATE_SCANNING;
    _scanStartMs = millis();
    Meter.setMode(METER_MODE_SCANNING);

    if (_currentProto == PROTO_WIFI) {
        int16_t status = WiFi.scanComplete();
        if (status != WIFI_SCAN_RUNNING) {
            WiFi.scanDelete();
            WiFi.scanNetworks(true, true); // Async all channels
        }
    } else {
        if (_pBLEScan) {
            _pBLEScan->clearResults();
            _pBLEScan->start(0, nullptr, false); // Continuous async scan
        }
    }
}

void Scanner::stopScan() {
    if (_state != SCAN_STATE_SCANNING) return;
    _state = SCAN_STATE_IDLE;
    Meter.setMode(METER_MODE_IDLE);

    if (_currentProto == PROTO_WIFI) {
        WiFi.scanDelete();
    } else {
        if (_pBLEScan) {
            _pBLEScan->stop();
        }
    }
}

void Scanner::selectTarget(int index) {
    portENTER_CRITICAL(&_mux);
    if (index >= 0 && index < (int)_targets.size()) {
        _trackedTarget = _targets[index];
    }
    portEXIT_CRITICAL(&_mux);

    selectTargetByAddress(_trackedTarget.address);
}

void Scanner::selectTargetByAddress(const String &address) {
    stopScan();

    _state = SCAN_STATE_TRACKING;
    _currentRssi = -100;
    _peakRssi = -100;
    _minRssi = 0;
    _smoothedRssi = -95.0f;
    _signalPercent = 0.0f;
    _packetCount = 0;
    _windowPackets = 0;
    _packetsPerSec = 0.0f;
    _lastPacketMs = millis();
    _rateWindowMs = millis();
    _lastHistoryPushMs = millis();

    for (int i = 0; i < HISTORY_LEN; i++) {
        _rssiHistory[i] = _minRssiThreshold;
    }

    Meter.setMode(METER_MODE_TRACKING);
    Meter.setSignal(_currentRssi, _signalPercent);

    if (_currentProto == PROTO_WIFI) {
        WiFi.scanDelete();
        _lastWifiScanTriggerMs = 0;
        // Fast targeted scan on AP's specific channel
        WiFi.scanNetworks(true, true, false, 55, _trackedTarget.channel);
    } else {
        if (_pBLEScan) {
            _pBLEScan->clearResults();
            _pBLEScan->start(0, nullptr, false); // Continuous tracking
        }
    }
}

void Scanner::stopTracking() {
    if (_state != SCAN_STATE_TRACKING) return;
    _state = SCAN_STATE_IDLE;
    Meter.setMode(METER_MODE_IDLE);

    if (_currentProto == PROTO_WIFI) {
        WiFi.scanDelete();
    } else {
        if (_pBLEScan) {
            _pBLEScan->stop();
        }
    }
}

int Scanner::getTargetCount() const {
    portENTER_CRITICAL((portMUX_TYPE*)&_mux);
    int count = _targets.size();
    portEXIT_CRITICAL((portMUX_TYPE*)&_mux);
    return count;
}

bool Scanner::getTarget(int index, TargetDevice &out) const {
    portENTER_CRITICAL((portMUX_TYPE*)&_mux);
    if (index >= 0 && index < (int)_targets.size()) {
        out = _targets[index];
        portEXIT_CRITICAL((portMUX_TYPE*)&_mux);
        return true;
    }
    portEXIT_CRITICAL((portMUX_TYPE*)&_mux);
    return false;
}

void Scanner::onResult(BLEAdvertisedDevice advertisedDevice) {
    if (_currentProto != PROTO_BLE) return;

    BLEAddress addr = advertisedDevice.getAddress();
    char addrBuf[24];
    snprintf(addrBuf, sizeof(addrBuf), "%s", addr.toString().c_str());
    int rssi = advertisedDevice.getRSSI();

    if (_state == SCAN_STATE_SCANNING) {
        portENTER_CRITICAL(&_mux);
        bool found = false;
        for (auto &t : _targets) {
            if (t.address.equalsIgnoreCase(addrBuf)) {
                t.rssi = rssi;
                t.lastSeenMs = millis();
                if (t.name.startsWith("BLE_")) {
                    String parsedName = extractBleName(advertisedDevice);
                    if (parsedName.length() > 0) t.name = parsedName;
                }
                found = true;
                break;
            }
        }

        if (!found && _targets.size() < 50) {
            TargetDevice dev;
            dev.proto = PROTO_BLE;
            dev.address = String(addrBuf);
            dev.rssi = rssi;
            dev.channel = 37;
            dev.lastSeenMs = millis();

            String parsedName = extractBleName(advertisedDevice);
            if (parsedName.length() > 0) {
                dev.name = parsedName;
            } else {
                dev.name = "BLE_" + dev.address.substring(dev.address.length() - 5);
            }
            dev.extra = "BLE Adv";
            _targets.push_back(dev);
        }
        portEXIT_CRITICAL(&_mux);
    } else if (_state == SCAN_STATE_TRACKING) {
        if (strcasecmp(addrBuf, _trackedTarget.address.c_str()) == 0) {
            recordRssiSample(rssi);
        }
    }
}

void Scanner::recordRssiSample(int rssi) {
    _currentRssi = rssi;
    if (_currentRssi > _peakRssi) _peakRssi = _currentRssi;
    if (_minRssi == 0 || _currentRssi < _minRssi) _minRssi = _currentRssi;

    // Exponential moving average filter
    _smoothedRssi = (_smoothedRssi * 0.65f) + (_currentRssi * 0.35f);
    _signalPercent = calculatePercent((int)(_smoothedRssi + 0.5f));

    _lastPacketMs = millis();
    _packetCount++;
    _windowPackets++;

    Meter.setSignal(_currentRssi, _signalPercent);
}

float Scanner::calculatePercent(int rssi) {
    if (rssi <= _minRssiThreshold) return 0.0f;
    if (rssi >= _maxRssiThreshold) return 100.0f;
    return ((float)(rssi - _minRssiThreshold) / (float)(_maxRssiThreshold - _minRssiThreshold)) * 100.0f;
}

int8_t Scanner::getHistory(int index) const {
    if (index >= 0 && index < HISTORY_LEN) {
        return _rssiHistory[index];
    }
    return _minRssiThreshold;
}

void Scanner::adjustMinRssi(int delta) {
    _minRssiThreshold = constrain(_minRssiThreshold + delta, -110, _maxRssiThreshold - 10);
    _signalPercent = calculatePercent(_currentRssi);
    Meter.setSignal(_currentRssi, _signalPercent);
}

void Scanner::adjustMaxRssi(int delta) {
    _maxRssiThreshold = constrain(_maxRssiThreshold + delta, _minRssiThreshold + 10, -10);
    _signalPercent = calculatePercent(_currentRssi);
    Meter.setSignal(_currentRssi, _signalPercent);
}

void Scanner::resetMetrics() {
    _peakRssi = _currentRssi;
    _minRssi = _currentRssi;
    _packetCount = 0;
    _windowPackets = 0;
    for (int i = 0; i < HISTORY_LEN; i++) {
        _rssiHistory[i] = _currentRssi;
    }
}

void Scanner::updateWiFiScan() {
    int16_t n = WiFi.scanComplete();
    if (n >= 0) {
        portENTER_CRITICAL(&_mux);
        _targets.clear();
        for (int i = 0; i < n; i++) {
            TargetDevice dev;
            dev.proto = PROTO_WIFI;
            dev.name = WiFi.SSID(i);
            if (dev.name.length() == 0) dev.name = "<Hidden SSID>";
            dev.address = WiFi.BSSIDstr(i);
            dev.address.toUpperCase();
            dev.rssi = WiFi.RSSI(i);
            dev.channel = WiFi.channel(i);
            dev.extra = authModeToString(WiFi.encryptionType(i));
            dev.lastSeenMs = millis();
            _targets.push_back(dev);
        }

        // Sort by RSSI descending (strongest AP at top)
        std::sort(_targets.begin(), _targets.end(), [](const TargetDevice &a, const TargetDevice &b) {
            return a.rssi > b.rssi;
        });
        portEXIT_CRITICAL(&_mux);

        WiFi.scanDelete();
        _state = SCAN_STATE_IDLE;
        Meter.setMode(METER_MODE_IDLE);
    } else if (n == WIFI_SCAN_FAILED) {
        _state = SCAN_STATE_IDLE;
        Meter.setMode(METER_MODE_IDLE);
    }
}

void Scanner::updateWiFiTracking() {
    int16_t n = WiFi.scanComplete();
    unsigned long now = millis();

    if (n >= 0) {
        for (int i = 0; i < n; i++) {
            String bssid = WiFi.BSSIDstr(i);
            bssid.toUpperCase();
            if (bssid.equalsIgnoreCase(_trackedTarget.address)) {
                recordRssiSample(WiFi.RSSI(i));
                break;
            }
        }
        WiFi.scanDelete();

        // Immediately trigger next rapid scan on the target channel
        WiFi.scanNetworks(true, true, false, 55, _trackedTarget.channel);
        _lastWifiScanTriggerMs = now;
    } else if (n == -2) {
        // No scan in progress, kick off targeted scan
        if (now - _lastWifiScanTriggerMs > 60) {
            WiFi.scanNetworks(true, true, false, 55, _trackedTarget.channel);
            _lastWifiScanTriggerMs = now;
        }
    }
}

void Scanner::updateBLEScan() {
    unsigned long now = millis();
    static unsigned long lastSortMs = 0;

    // Periodically sort discovered BLE devices by RSSI
    if (now - lastSortMs > 800) {
        lastSortMs = now;
        portENTER_CRITICAL(&_mux);
        std::sort(_targets.begin(), _targets.end(), [](const TargetDevice &a, const TargetDevice &b) {
            return a.rssi > b.rssi;
        });
        portEXIT_CRITICAL(&_mux);
    }

    // Automatically stop active scanning after 6 seconds of discovery
    if (_state == SCAN_STATE_SCANNING && (now - _scanStartMs > 6000)) {
        stopScan();
        portENTER_CRITICAL(&_mux);
        std::sort(_targets.begin(), _targets.end(), [](const TargetDevice &a, const TargetDevice &b) {
            return a.rssi > b.rssi;
        });
        portEXIT_CRITICAL(&_mux);
    }
}

void Scanner::updateBLETracking() {
    // Tracking is handled dynamically by onResult() callback
}

void Scanner::update() {
    unsigned long now = millis();

    // 1. Radio state machines
    if (_currentProto == PROTO_WIFI) {
        if (_state == SCAN_STATE_SCANNING) {
            updateWiFiScan();
        } else if (_state == SCAN_STATE_TRACKING) {
            updateWiFiTracking();
        }
    } else { // BLE
        if (_state == SCAN_STATE_SCANNING) {
            updateBLEScan();
        } else if (_state == SCAN_STATE_TRACKING) {
            updateBLETracking();
        }
    }

    // 2. Compute packet arrival rate (packets/sec)
    if (now - _rateWindowMs >= 1000) {
        _packetsPerSec = (_windowPackets * 1000.0f) / (float)(now - _rateWindowMs);
        _windowPackets = 0;
        _rateWindowMs = now;
    }

    // 3. Signal decay if no packets received for > 1800ms
    if (_state == SCAN_STATE_TRACKING && (now - _lastPacketMs > 1800)) {
        _currentRssi = max(_minRssiThreshold, _currentRssi - 1);
        _smoothedRssi = (_smoothedRssi * 0.9f) + (_currentRssi * 0.1f);
        _signalPercent = calculatePercent((int)_smoothedRssi);
        Meter.setSignal(_currentRssi, _signalPercent);
    }

    // 4. Push to 2D Oscilloscope / Waterfall History Buffer (~4 times a second)
    if (_state == SCAN_STATE_TRACKING && (now - _lastHistoryPushMs >= 250)) {
        _lastHistoryPushMs = now;
        for (int i = 0; i < HISTORY_LEN - 1; i++) {
            _rssiHistory[i] = _rssiHistory[i + 1];
        }
        _rssiHistory[HISTORY_LEN - 1] = (int8_t)constrain(_currentRssi, -120, 0);
    }
}
