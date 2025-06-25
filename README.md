# ESP32 OTA Firmware Update System

This project implements a fully functional Over-The-Air (OTA) firmware update system using two ESP32 boards with the ESP-IDF framework — no Arduino.

One ESP32 hosts a local HTTP server (`OTA_Controller`) serving the firmware binary from SPIFFS. The other ESP32 (`OTA_Client`) listens for a network-based trigger and performs a secure OTA update using ESP-IDF's low-level APIs.

---

## Features

- Bare-metal ESP-IDF (no Arduino dependencies)
- Local HTTP server (`esp_http_server`) for serving firmware
- SPIFFS file system for OTA binary storage (`ota1.bin`)
- Firmware flashing using `esp_ota_ops`
- Custom flash partition table to support OTA slots

---

## What I Learned

- How to host static binary content on ESP32 via SPIFFS
- How OTA flashing and rollback-safe commits work in ESP-IDF
- Building multi-ESP systems using WiFi, HTTP, and sockets
- Deepened understanding of NVS, flash partitions, and task synchronization
- Working with FreeRTOS and low-level C on real hardware

---

## Limitations

This is a proof-of-concept and deliberately omits certain production safeguards:

- No HTTPS (served over HTTP only)
- No firmware signature or image validation
- Hardcoded IP address of controller — no device discovery

In a real-world deployment, use TLS (`esp_tls`) and signed firmware to protect against man-in-the-middle or unauthorized updates.

---

## How to Run

1. **Flash OTA_Controller to ESP32 #1**  
   This device runs a local HTTP server and serves `ota1.bin` via SPIFFS.

2. **Flash OTA_Client to ESP32 #2**  
   This device automatically downloads and applies the update on boot using `esp_https_ota()`.

3. **Modify IP Address**  
   In `main.c` of the client, update `OTA_URL` to match the controller’s IP address.

4. **Connect Both Devices to the Same Wi-Fi**

5. **Observe OTA Logs**  
   Monitor serial output of the OTA client to verify firmware download and reboot.

## Future Improvements
- Enable OTA over HTTPS using esp_tls

- Add external trigger (button, UDP packet, or web dashboard) to initiate OTA dynamically

- Build a web dashboard to manage and trigger updates  

- Implement RSA signature verification

- Auto-detect controller via mDNS or DHCP fallback

- Add firmware versioning and rollback logic



## Author
Cale Conrad

Electrical Engineering – University of Nebraska-Lincoln

