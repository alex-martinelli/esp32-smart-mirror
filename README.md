# ESP32 Smart Mirror Device

![C/C++](https://img.shields.io/badge/C%2FC%2B%2B-Embedded_Systems-00599C?logo=cplusplus)
![ESP32](https://img.shields.io/badge/ESP32-Microcontroller-E7352C?logo=espressif)
![Hardware](https://img.shields.io/badge/Hardware-IoT_Integration-success)

Embedded C/C++ firmware for a custom-built physical smart mirror powered by an ESP32 microcontroller, integrating environmental sensors and an active TFT display.

## 📌 Project Overview
The project consists of the hardware assembly and low-level software engineering of an interactive mirror. The system automatically triggers the display and user interface overlays upon detecting physical presence, providing real-time environmental data alongside dynamic visual content.

## ⚙️ Key Features & Architecture
* **Hardware Integration:** Interfaced the ESP32 with an ultrasonic distance sensor (HC-SR04), a DHT11 temperature/humidity sensor, a micro SD module (via HSPI), and a TFT SPI display.
* **Non-Blocking Logic:** Programmed the embedded system utilizing a state-driven approach (`millis()`) to manage sensor readings and display activation without halting the main microcontroller execution loop.
* **Dynamic Memory Management:** Optimized hardware performance by implementing dynamic RAM allocation (`malloc`/`free`) and sequential loading for JPEG decoding. This strategy halves the peak memory requirement, successfully preventing ESP32 heap exhaustion during image rendering.

## 🚀 How to Run / Hardware Setup
1. Clone the repository: `git clone https://github.com/alex-martinelli/esp32-smart-mirror.git`
2. Open the project in the Arduino IDE or PlatformIO.
3. Ensure the following libraries are installed: `TFT_eSPI`, `DHT sensor library`, `JPEGDecoder`.
4. Connect the hardware components according to the pin definitions in the source code (e.g., Ultrasonic Trigger on Pin 26, DHT on Pin 32).
5. Load the required `.jpg` assets onto the root of a FAT32-formatted micro SD card.
6. Compile and upload the firmware to the ESP32 board.
