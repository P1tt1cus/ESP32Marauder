#!/usr/bin/env python3
"""Executable checks for the board profile and unsupported-command contract."""
import pathlib
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]

class StudioProfileTests(unittest.TestCase):
    def test_profile_clears_unfitted_peripherals(self):
        unsupported = ['HAS_SD', 'USE_SD', 'HAS_NEOPIXEL_LED', 'HAS_SCREEN', 'HAS_BUTTONS',
                       'HAS_BATTERY', 'HAS_PWR_MGMT', 'HAS_TEMP_SENSOR', 'HAS_GPS',
                       'HAS_PSRAM', 'HAS_DIRECT_UPLOAD']
        source = '#define STUDIO_ZY_ESP32\n#define HAS_BT\n#define HAS_CSI_MOTION\n'
        source += ''.join('#define '+name+'\n' for name in unsupported)
        source += '#include "StudioProfile.h"\n'
        source += ''.join('#ifdef '+name+'\n#error Unexpected peripheral\n#endif\n' for name in unsupported)
        source += '#if !defined(HAS_BT) || !defined(HAS_CSI_MOTION) || !defined(HAS_SERIAL_CAPTURE)\n#error Missing supported feature\n#endif\n'
        subprocess.run(['c++', '-x', 'c++', '-fsyntax-only', '-I', str(ROOT/'esp32_marauder'), '-'], input=source, text=True, check=True)

    def test_command_contract(self):
        source = r'''
#include "StudioSupport.h"
#include <assert.h>
int main() {
 const char* blocked[] = {"gps", "gpsdata", "gpstracker", "gpspoi", "nmea", "wardrive", "wardrivepoi", "geofence", "upload", "ls", "save", "load", "update", "backupspiffs", "backupstatus", "restorespiffs", "led", "brightness"};
 for(const char* cmd : blocked) assert(marauder::studioUnsupportedReason(cmd));
 const char* kept[] = {"protocolinfo", "diagnostics", "sniffbeacon", "sniffraw", "sniffbt", "csimotion", "recon", "remoteid", "channel", "stopscan", "join", "list"};
 for(const char* cmd : kept) assert(!marauder::studioUnsupportedReason(cmd));
 assert(marauder::studioUnsupportedReason("evilportal", "-c", "sethtml"));
 assert(!marauder::studioUnsupportedReason("evilportal", "-c", "sethtmlstr"));
 assert(!marauder::studioUnsupportedReason("evilportal", "-c", "start"));
 assert(marauder::studioUnsupportedReason("settings", "-s", "EnableLED"));
 assert(!marauder::studioUnsupportedReason("settings", "-s", "SavePCAP"));
 assert(!marauder::studioUnsupportedReason("gpsnotacommand"));
 assert(!marauder::studioUnsupportedReason(nullptr));
}
'''
        with tempfile.TemporaryDirectory() as directory:
            src=pathlib.Path(directory)/'contract.cpp';binary=pathlib.Path(directory)/'contract';src.write_text(source)
            subprocess.run(['c++','-std=c++17','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-I',str(ROOT/'esp32_marauder'),str(src),str(ROOT/'esp32_marauder/StudioSupport.cpp'),'-o',str(binary)],check=True)
            subprocess.run([str(binary)],check=True)

if __name__ == '__main__': unittest.main()
