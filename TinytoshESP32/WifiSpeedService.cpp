#include "WifiSpeedService.h"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

bool WifiSpeedService::runTest(WifiSpeedData &data) {
    Serial.println("WifiSpeedService: Running speed test...");

    WiFiClientSecure secureClient;
    secureClient.setInsecure();

    HTTPClient http;
    http.setReuse(false);
    http.setConnectTimeout(5000);
    http.setTimeout(DOWNLOAD_TIMEOUT_MS);

    unsigned long requestStart = millis();
    if (!http.begin(secureClient, SPEED_TEST_URL)) {
        Serial.println("WifiSpeedService: Failed to initialize HTTP client.");
        return false;
    }

    int httpCode = http.GET();
    unsigned long ttfb = millis() - requestStart;

    if (httpCode != HTTP_CODE_OK) {
        Serial.printf("WifiSpeedService: Request failed, HTTP Code: %d\n", httpCode);
        http.end();
        return false;
    }

    WiFiClient* stream = http.getStreamPtr();
    uint8_t buf[512];
    size_t received = 0;
    unsigned long downloadStart = millis();

    while (http.connected() && (millis() - downloadStart < DOWNLOAD_TIMEOUT_MS)) {
        size_t avail = stream->available();
        if (avail > 0) {
            size_t toRead = min(avail, sizeof(buf));
            received += stream->readBytes(buf, toRead);
        } else if (!stream->connected()) {
            break;
        }
    }
    unsigned long downloadMs = millis() - downloadStart;
    http.end();

    if (received == 0 || downloadMs == 0) {
        Serial.println("WifiSpeedService: No data received.");
        return false;
    }

    double seconds = downloadMs / 1000.0;
    data.download_mbps = (received * 8.0) / (seconds * 1000000.0);
    data.ping_ms = (int)ttfb;
    data.last_update_ms = millis();
    data.updated = true;

    Serial.printf("WifiSpeedService: Success! %.2f Mbps, TTFB: %lums, Bytes: %u\n",
                  data.download_mbps, ttfb, (unsigned)received);
    return true;
}
