# Studio ESP32 firmware maintenance

This is P1tt1cus's maintained ESP32 Marauder fork. Read the Studio section of README.md.

- `origin`: https://github.com/P1tt1cus/ESP32Marauder
- `upstream`: https://github.com/justcallmekoko/ESP32Marauder
- Maintained/default branch: `studio`. Start feature branches here and target PRs here.
- Base: v1.20.0, commit `511bb3694d0c4dc064888e762bde44fcc9b7f48a`.
- Our board is ZY-ESP32, ESP32-D0WD-V3, 4 MB flash. Use `STUDIO_ZY_ESP32` (inherits LDDB);
  do not substitute the official Flipper ESP32-S2 devboard target.
- Profile and upstream build recipe are recorded in `studio-target.json`.
  `platformio.ini` is the native test environment, not the LDDB firmware build.
- UART0: 115200, GPIO3 RX / GPIO1 TX. Flipper pin13 TX -> GPIO3;
  pin14 RX -> GPIO1; pin1 5V -> VIN; pin11 -> GND. Power off before rewiring.
- Baseline LDDB has no HAS_GPS support or configured GPS module. Missing ESP SD
  is expected. The companion saves to Flipper SD separately.
- Paired repositories: P1tt1cus/flipperzero-wifi-marauder and P1tt1cus/Momentum-Firmware.
  Keep the machine protocol compatible, or update and test both ends together.
- Read test/README.md for native tests. Build the Studio target for relevant changes;
  distinguish compile success, host tests, and actual hardware evidence.
- Fetch and review upstream on an integration branch. Never reset our branch to
  upstream or automatically merge upstream releases. Preserve MIT attribution.
- Never commit Wi-Fi credentials, captures, device dumps, secrets, or build caches.
- Studio changes strip unsupported peripherals and add capability/diagnostic replies.
  Initial fork setup did not flash devices; builds do not imply hardware verification.
  Preserve the installed image and data when planning subsequent firmware upgrades.
