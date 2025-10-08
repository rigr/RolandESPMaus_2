# RolandESPMaus

Use an ESP32 to allow Bluetooth mice to control a Roland S-750 sampler.  
This repository builds a firmware image (ESP-IDF) that:

- acts as a Bluetooth HID Host, automatically connecting the strongest mouse (by RSSI),
- emulates the Roland/MSX nibble-based mouse protocol on the DB9 mouse port (requires 3.3↔5V level shifter),
- runs a Wi-Fi AP `RolandMouse` (password `MausMaus`) with a simple web UI,
- shows status on serial every 5s and accepts commands (`scan`, `scan WiFi`, `help`),
- toggles on-board LED: blink = no mouse, steady = mouse connected.

---

## Repository files to check
- `CMakeLists.txt` (root) — minimal esp-idf project file
- `main/CMakeLists.txt` — registers the main component
- `main/main.c` — application code (replace with the IDF-v5.x compatible `main.c` provided)
- `.github/workflows/build.yml` — CI build workflow (provided)
- `sdkconfig.defaults` — default build configuration

---

## Build (via GitHub Actions, automatic)
1. Push your code to `main` branch on GitHub.
2. GitHub Actions (workflow `.github/workflows/build.yml`) will run automatically.
3. After a successful build, go to **Actions → Build RolandESPMaus Firmware → Artifacts** and download `RolandESPMaus-Firmware.zip` (contains `*.bin`).

---

## Flashing (minimal local setup)
You only need `esptool.py` locally to flash the binary downloaded from GitHub:

1. Install `esptool.py`:
```bash
pip install esptool
```
2. Put the ESP into bootloader mode and flash (example):
```bash
esptool.py --chip esp32 --port /dev/ttyUSB0 write_flash -z 0x1000 build/rolandespmaus.bin
```
(adjust filename and port accordingly)
Alternative: use idf.py -p <port> flash monitor if you have esp-idf locally.

## Serial console
Baudrate: 115200
Commands:
help — show commands
scan — start BT HID discovery (selects strongest RSSI device)
scan WiFi — scan for Wi-Fi networks
show — show current connection status

## Web UI
Connect to Wi-Fi SSID RolandMouse (password MausMaus).
Open http://192.168.4.1/ (AP default IP). The web page shows connection status and allows triggering BT scans.

## Hardware wiring (critical)
Do not connect Roland's DB9 5V lines directly to the ESP32.
Use a bidirectional 3.3V ↔ 5V level shifter (8-channel) for all DB9 signal lines:
DB9 pins 1..4 (data), pin6/7 (buttons/triggers), pin8 (strobe) — all through level shifter.
Connect Roland GND and ESP32 GND together.
Power the ESP from the Roland supply only if you verify proper voltage regulation; safer: power via USB during testing.

##Troubleshooting
If CI fails with idf.py: not found, ensure workflow is using espressif/setup-esp-idf@v2.
If build errors relate to esp_hidh API: this repo expects ESP-IDF v5.1. The workflow pins that version.
If mouse reports have unexpected layout (many mice use different report formats), adapt HID report parsing in main.c (RPT handler).

##License & Notes
This is a hardware modification project — proceed at your own risk.
No persistent pairing is written to flash; the connection lives in RAM until power-off.
