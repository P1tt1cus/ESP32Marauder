#pragma once

// Our verified four-wire ZY-ESP32 setup. Apply after the LDDB defaults.
#ifdef STUDIO_ZY_ESP32
#undef HAS_SD
#undef USE_SD
#undef HAS_NEOPIXEL_LED
#undef HAS_SCREEN
#undef HAS_BUTTONS
#undef HAS_BATTERY
#undef HAS_PWR_MGMT
#undef HAS_TEMP_SENSOR
#undef HAS_GPS
#undef HAS_PSRAM
#undef HAS_DIRECT_UPLOAD
#define HAS_SERIAL_CAPTURE
#define STUDIO_PROFILE_ID "studio-zy-esp32"
#define STUDIO_BUILD_ID "studio.2-au"
#endif
