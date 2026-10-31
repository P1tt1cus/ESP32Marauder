#pragma once
#include <stddef.h>
#include <stdint.h>
#ifdef STUDIO_ZY_ESP32
#include "esp_wifi.h"
namespace marauder {
constexpr uint8_t kStudioFirstChannel = 1;
constexpr uint8_t kStudioLastChannel = 13;
// Driver conducted-power ceiling, not an EIRP measurement or legal certification.
constexpr int8_t kStudioTxPowerQdbm = 80;
esp_err_t studioRadioApply();
bool studioRadioRunning();
esp_err_t studioRadioValidateChannel(int channel);
// Read-only driver snapshot. These functions never initialize/start a radio.
bool studioRadioJson(char* output, size_t size);
void studioRadioPrint();
}
#endif
