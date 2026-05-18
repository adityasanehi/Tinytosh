#include "CalendarService.h"

CalendarService::CalendarService() {}

void CalendarService::fetchHolidays(const String& countryCode, CalendarData& data) {
    if (countryCode == "") {
        Serial.println("CalendarService: No country code set. Skipping fetch.");
        return;
    }

    time_t now = time(nullptr);
    struct tm timeinfo;
    if (!localtime_r(&now, &timeinfo) || timeinfo.tm_year < 120) { 
        Serial.println("CalendarService: Time not synced yet. Cannot determine year.");
        return;
    }
    int currentYear = timeinfo.tm_year + 1900;

    String url = String(API_BASE_URL) + String(currentYear) + "/" + countryCode;
    Serial.println("CalendarService: Fetching holidays from " + url);

    WiFiClientSecure client;
    client.setInsecure(); 
    
    HTTPClient http;
    http.begin(client, url);
    http.setTimeout(10000);
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();

        DynamicJsonDocument doc(8192);
        DeserializationError error = deserializeJson(doc, payload);

        if (!error) {
            JsonArray arr = doc.as<JsonArray>();
            data.count = 0;
            
            for (JsonObject v : arr) {
                if (data.count >= 30) break;
                
                data.items[data.count].date = v["date"].as<String>();
                data.items[data.count].name = v["name"].as<String>();
                data.count++;
            }
            
            data.updated = true;
            Serial.printf("CalendarService: Successfully loaded %d holidays.\n", data.count);
        } else {
            Serial.print("CalendarService: JSON parsing failed: ");
            Serial.println(error.c_str());
        }
    } else {
        Serial.printf("CalendarService: HTTP GET failed, code: %d\n", httpCode);
    }
    
    http.end();
}