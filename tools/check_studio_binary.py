#!/usr/bin/env python3
"""Check linked firmware, not just feature macros, before publishing artifacts."""
import argparse
import pathlib
import subprocess

parser = argparse.ArgumentParser()
parser.add_argument('elf', type=pathlib.Path)
parser.add_argument('--nm', required=True)
args = parser.parse_args()
symbols = subprocess.check_output([args.nm, '-C', '--defined-only', str(args.elf)], text=True)
for forbidden in ('fs::SDFS::', 'SDInterface::', 'GPSInterface::', 'MicroNMEA::',
                  'Adafruit_NeoPixel::', 'TFT_eSPI::', 'Adafruit_MAX17048::',
                  'Settings::saveGeofence(', 'Settings::clearGeofence('):
    if forbidden in symbols:
        raise SystemExit('Unsupported hardware code linked: '+forbidden)
for required in ('Buffer::saveSerial()', 'NimBLEDevice::init(', 'CsiMotionMonitor::start(',
                 '__wrap_esp_wifi_init', '__wrap_esp_wifi_start', '__wrap_esp_wifi_stop',
                 '__wrap_esp_wifi_deinit', '__wrap_esp_wifi_set_channel',
                 '__wrap_esp_wifi_set_max_tx_power', '__wrap_esp_wifi_set_config'):
    if required not in symbols:
        raise SystemExit('Missing supported feature: '+required)
if b'studio-zy-esp32' not in args.elf.read_bytes():
    raise SystemExit('Missing Studio profile identity')
print('PASS: unsupported peripheral drivers absent; UART capture, BLE and CSI retained')
