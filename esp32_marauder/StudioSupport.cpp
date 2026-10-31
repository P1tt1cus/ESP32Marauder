#include "StudioSupport.h"
#include <string.h>

namespace marauder {
namespace {
bool equals(const char* a, const char* b) { return a && strcmp(a, b) == 0; }
}

bool studioSettingSupported(const char* name) {
  return !equals(name, "EnableLED") && !equals(name, "Geofences");
}

const char* studioUnsupportedReason(const char* command, const char* option, const char* value) {
  const char* gps[] = {"gps", "gpsdata", "gpstracker", "gpspoi", "nmea", "wardrive", "wardrivepoi", "geofence", "upload"};
  for(const char* name : gps) if(equals(command, name)) return "GPS is not fitted to this board";
  const char* sd[] = {"ls", "save", "load", "update", "backupspiffs", "backupstatus", "restorespiffs"};
  for(const char* name : sd) if(equals(command, name)) return "ESP32 SD hardware is not fitted; use Flipper sessions";
  if(equals(command, "led")) return "No configured RGB LED";
  if(equals(command, "brightness")) return "Use the Flipper display controls";
  if(equals(command, "settings") && equals(option, "-s") && !studioSettingSupported(value)) return "Setting is not supported by this board";
  if(equals(command, "evilportal") && equals(option, "-c") && equals(value, "sethtml")) return "ESP32 SD HTML files are unavailable; use serial HTML";
  return nullptr;
}
}
