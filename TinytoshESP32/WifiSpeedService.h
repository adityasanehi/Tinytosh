#ifndef WIFI_SPEED_SERVICE_H
#define WIFI_SPEED_SERVICE_H

#include "structs.h"

class WifiSpeedService {
public:
  bool runTest(WifiSpeedData &data);

private:
  static constexpr const char* SPEED_TEST_URL = "https://speed.cloudflare.com/__down?bytes=4000000";
  static const unsigned long DOWNLOAD_TIMEOUT_MS = 15000;
};

#endif
