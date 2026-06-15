#include "ShopifyService.h"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

bool ShopifyService::fetchSales(const Config& config, ShopifyData &data) {
    String url = config.shopify_url;
    url.trim();

    if (url.length() == 0) {
        Serial.println("ShopifyService: URL is empty, skipping.");
        return false;
    }

    Serial.printf("ShopifyService: Requesting sales data from %s\n", url.c_str());

    WiFiClientSecure secureClient;
    secureClient.setInsecure();

    HTTPClient http;
    http.setReuse(false);

    bool began = false;
    if (url.startsWith("https://")) {
        began = http.begin(secureClient, url);
    } else {
        began = http.begin(url);
    }

    if (!began) {
        Serial.println("ShopifyService: Failed to initialize HTTP client.");
        return false;
    }

    http.setConnectTimeout(10000);
    http.setTimeout(10000);

    int httpCode = http.GET();

    if (httpCode == 200) {
        String payload = http.getString();

        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, payload);

        if (!error) {
            if (!doc.containsKey("total_sales") || !doc.containsKey("currency") ||
                doc["total_sales"].isNull() || doc["currency"].isNull()) {
                Serial.println("ShopifyService: Required fields total_sales/currency missing.");
                http.end();
                return false;
            }

            String currency = doc["currency"].as<String>();
            currency.trim();
            if (currency.length() == 0) {
                Serial.println("ShopifyService: Currency is empty.");
                http.end();
                return false;
            }

            String store = doc["store"] | config.shopify_store_name.c_str();
            store.trim();
            if (store.length() == 0) store = "Shopify";

            String period = doc["period"] | "today";
            period.trim();
            if (period.length() == 0) period = "today";

            data.store = store;
            data.period = period;
            data.currency = currency;
            data.currency.toUpperCase();
            data.total_sales = doc["total_sales"].as<float>();
            data.order_count = doc["order_count"] | 0;
            data.percent_change = doc["percent_change"] | 0.0;
            data.updated = true;

            Serial.printf("ShopifyService: Success! %s %.2f %s Orders: %d Change: %+.2f%%\n",
                          data.store.c_str(), data.total_sales, data.currency.c_str(),
                          data.order_count, data.percent_change);

            http.end();
            return true;
        } else {
            Serial.printf("ShopifyService: JSON parsing failed: %s\n", error.c_str());
        }
    } else {
        Serial.printf("ShopifyService: Endpoint failed, HTTP Code: %d\n", httpCode);
    }

    http.end();
    return false;
}
