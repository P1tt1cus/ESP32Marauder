# Studio ESP32 firmware

Personal maintained fork: [P1tt1cus/ESP32Marauder](https://github.com/P1tt1cus/ESP32Marauder).
Our default branch is `studio`; `origin` is our fork and `upstream` is the original.
The custom profile is based on **v1.20.0 / 511bb3694d0c4dc064888e762bde44fcc9b7f48a**,
the release verified on our ZY-ESP32. Building does not flash the connected device.

## Our target

`studio-target.json` records **STUDIO_ZY_ESP32**, profile `studio-zy-esp32`, build
`studio.1`: ESP32-D0WD-V3, 4 MB flash, UART0 at 115200. It inherits LDDB's chip,
BLE and CSI configuration, then removes SD, GPS, RGB LED, screen/touch controls,
battery monitoring and PSRAM support. This is not the official ESP32-S2 board.
The existing no-PSRAM memory limits remain in place.

The paired [companion](https://github.com/P1tt1cus/flipperzero-wifi-marauder)
removes commands for the absent peripherals. Captures still stream over UART
with `-serial` to the **Flipper's SD**; keep `SavePCAP` enabled for capture output.
SPIFFS remains for settings. ESP-side SD list/save/load, backup/restore, SD updates,
GPS/wardrive/geofence/upload, LED and brightness commands explicitly return
`UNSUPPORTED`. Serial HTML loading remains supported; SD HTML paths do not.
Legacy settings/script fields can remain on disk for compatibility.

`protocolinfo --machine studio` reports profile, build, supported capabilities
and absent hardware. `diagnostics` reports chip, flash, heap, reset reason, scan
mode and uptime; `diagnostics --machine <tx>` returns structured protocol 1 JSON.
These commands do not start a radio scan.

Build with `.github/workflows/studio-build.yml`: Arduino ESP32 **3.3.4**, NimBLE
**2.3.8**, `esp32:esp32:d32:PartitionScheme=min_spiffs`. This follows the upstream
LDDB recipe with our build flag. `platformio.ini` runs native tests, not firmware.
CI checks the linked ELF for absent peripheral drivers and retained serial
capture/BLE/CSI, then publishes the application, bootloader, partition table,
merged image, ELF and map. Do not flash the merged image over existing data as a
routine upgrade; verify the device and partition layout first.

Run `python3 tools/test_studio_profile.py` and
`python3 tools/test_spiffs_machine_protocol.py` for host checks. Firmware build
success is separate from boot/UART verification on the physical board.

Initialize pinned library submodules with:

```sh
git submodule update --init --recursive
```

See `test/README.md` for native tests. Work on branches from `studio`, review
upstream updates separately, and validate changes against this board before
releasing. Preserve upstream license and attribution. Keep credentials,
captures, and personal device data out of source control.

---

## Upstream documentation

<!---[![License: MIT](https://img.shields.io/github/license/mashape/apistatus.svg)](https://github.com/justcallmekoko/ESP32Marauder/blob/master/LICENSE)--->
<!---[![Gitter](https://badges.gitter.im/justcallmekoko/ESP32Marauder.png)](https://gitter.im/justcallmekoko/ESP32Marauder)--->
<!---[![Build Status](https://travis-ci.com/justcallmekoko/ESP32Marauder.svg?branch=master)](https://travis-ci.com/justcallmekoko/ESP32Marauder)--->
<!---Shields/Badges https://shields.io/--->

# ESP32 Marauder
<p align="center"><img alt="Marauder logo" src="https://github.com/justcallmekoko/ESP32Marauder/blob/master/pictures/marauder_skull_patch_04_full_final.png?raw=true" width="300"></p>
<p align="center">
  <b>A suite of WiFi/Bluetooth offensive and defensive tools for the ESP32</b>
  <br><br>
  <a href="https://github.com/justcallmekoko/ESP32Marauder/blob/master/LICENSE"><img alt="License" src="https://img.shields.io/github/license/mashape/apistatus.svg"></a>
  <a href="https://gitter.im/justcallmekoko/ESP32Marauder"><img alt="Gitter" src="https://badges.gitter.im/justcallmekoko/ESP32Marauder.png"/></a>
  <br>
  <a href="https://twitter.com/intent/follow?screen_name=jcmkyoutube"><img src="https://img.shields.io/twitter/follow/jcmkyoutube?style=social&logo=twitter" alt="Twitter"></a>
  <a href="https://www.instagram.com/just.call.me.koko"><img src="https://img.shields.io/badge/Follow%20Me-Instagram-orange" alt="Instagram"/></a>
  <br><br>
</p>
    
[![Build and Push](https://github.com/justcallmekoko/ESP32Marauder/actions/workflows/build_push.yml/badge.svg)](https://github.com/justcallmekoko/ESP32Marauder/actions/workflows/build_push.yml)
[![Firmware coverage](https://codecov.io/gh/justcallmekoko/ESP32Marauder/branch/develop/graph/badge.svg?flag=firmware-unit-tests)](https://app.codecov.io/gh/justcallmekoko/ESP32Marauder/tree/develop)

## Getting Started
Download the [latest release](https://github.com/justcallmekoko/ESP32Marauder/releases/latest) of the firmware.  

Check out the project [wiki](https://github.com/justcallmekoko/ESP32Marauder/wiki) for a full overview of the ESP32 Marauder

## FCC module identifiers

The Device Info screen reports the FCC ID of the radio module fitted to supported JustCallMeKoko hardware. This is the module identifier, not a claim that every carrier-board or enclosure configuration has a separate end-product grant.

| Hardware | Radio module | FCC ID |
| --- | --- | --- |
| Marauder v8, Marauder Mini v3, C5 Wardriver, C5 LowKey, BFFB v2 | ESP32-C5-WROOM-U | `2AC7Z-ESPC5WROOMU` |
| Marauder v4, v6, v6.1, v7, original Mini, Marauder Kit | ESP32-WROOM-32U | `2AC7Z-ESP32WROOM32U` |
| BFFB / Marauder Dev Board Pro | ESP32-WROOM-32E | `2AC7Z-ESP32WROOM32E` |

The legacy mappings are the closest modular approvals for the modules specified by the repository hardware designs. Confirm the module marking on a physical unit before using an FCC ID for labeling or compliance documentation. Third-party targets display `See hardware label` because their module or complete-device certification can vary by production revision.

# For Sale Now
You can buy the ESP32 Marauder using [this link](https://www.justcallmekokollc.com)
