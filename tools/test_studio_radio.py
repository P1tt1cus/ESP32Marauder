#!/usr/bin/env python3
"""Compile the production driver boundary with controllable ESP-IDF test doubles."""
import json
import pathlib
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]

class StudioRadioTests(unittest.TestCase):
    def test_driver_contract(self):
        with tempfile.TemporaryDirectory() as directory:
            binary = pathlib.Path(directory) / 'radio_test'
            subprocess.run(['c++', '-std=c++17', '-Wall', '-Wextra', '-Werror',
                            '-fsanitize=address,undefined', '-DSTUDIO_ZY_ESP32', '-DSTUDIO_RADIO_WRAPPERS',
                            '-I', str(ROOT/'tools/test_support/studio_radio'), '-I', str(ROOT/'esp32_marauder'),
                            str(ROOT/'esp32_marauder/StudioRadio.cpp'), str(ROOT/'tools/test_support/studio_radio/radio_test.cpp'),
                            '-o', str(binary)], check=True)
            output = subprocess.check_output([str(binary)], text=True)
            snapshots = [json.loads(line) for line in output.splitlines() if line.startswith('{')]
            self.assertTrue(snapshots[0]['verified'])
            self.assertEqual(snapshots[0]['country'], 'AU')
            self.assertEqual(snapshots[0]['channel'], 13)
            self.assertEqual(snapshots[0]['powerQdbm'], 72)
            self.assertFalse(snapshots[1]['active'])
            self.assertIsNone(snapshots[1]['country'])
            self.assertGreater(snapshots[1]['errors'], snapshots[0]['errors'])
            print(output.splitlines()[-1])

if __name__ == '__main__': unittest.main()
