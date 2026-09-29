# Interactive Digital Photo Frame with ESP32

A digital photo frame based on an **ESP32** that wakes up when someone walks by.
An ultrasonic sensor detects presence, and the TFT display shows one or two photos from an SD card, overlaid with a random message and the current temperature and humidity.

## How it works

1. Every 500 ms the ultrasonic sensor measures the distance.
2. If the distance is **below 65 cm**, the display turns on:
   - 50% of the time it shows **one landscape photo** at full screen (480x320);
   - 50% of the time it shows **two portrait photos** side by side (240x320 each), always different from each other.
3. On top of the images it draws:
   - a **random phrase** in the top-left corner (red, with automatic word wrapping);
   - **temperature and humidity** (DHT11) in the bottom-right corner.
4. When nobody has been detected for **10 seconds**, the screen is cleared and the backlight is turned off.

> Note: the phrase and the DHT11 readings are drawn only if the sensor read succeeds.
> If the DHT11 does not respond, only the photos are shown.

## Hardware

- ESP32 (e.g. ESP32 Dev Module)
- 480x320 SPI TFT display (e.g. ILI9488 or ST7796 driver — **TODO: specify the model used**)
- microSD card module (SPI)
- Ultrasonic sensor (HC-SR04 type)
- DHT11 temperature and humidity sensor

## Wiring

| Component | Signal | ESP32 pin |
|---|---|---|
| Ultrasonic sensor | TRIG | GPIO 26 |
| Ultrasonic sensor | ECHO | GPIO 35 |
| DHT11 | DATA | GPIO 32 |
| TFT backlight (PWM) | LED | GPIO 2 |
| SD module | SCK | GPIO 25 |
| SD module | MISO | GPIO 33 |
| SD module | MOSI | GPIO 13 |
| SD module | CS | GPIO 4 |
| TFT display | see below | **TODO** |

**Separate SPI buses.** The SD card uses the **HSPI** bus with the pins above. The TFT display uses the **VSPI** bus (ESP32 default pins: SCK 18, MISO 19, MOSI 23), handled by TFT_eSPI.

**Mind the ECHO pin.** The ESP32 runs at 3.3 V. If your sensor is a 5 V HC-SR04, the ECHO output should be stepped down with a voltage divider (for example 1 kΩ + 2 kΩ) or you should use a 3.3 V version of the sensor.

## Required libraries

Install these from the Arduino Library Manager:

- [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) (Bodmer)
- [JPEGDecoder](https://github.com/Bodmer/JPEGDecoder) (Bodmer)
- DHT sensor library (Adafruit) and Adafruit Unified Sensor

Bundled with the ESP32 core: `SPI`, `SD`, `FS`.

**Required ESP32 core:** version **3.x** (the sketch uses `ledcAttach()`, which does not exist in 2.x).

## Display configuration (TFT_eSPI)

TFT_eSPI does not read its pins from the sketch, but from the `User_Setup.h` file inside the library folder.
The [`config/`] folder of this repository contains a copy of the file used for this project: replace the library's own file with it (`Documents/Arduino/libraries/TFT_eSPI/User_Setup.h`) before compiling.

> **TODO:** add `config/User_Setup.h` to the repository.

The display is initialized with `setRotation(1)` (landscape, 480x320).

## Preparing the SD card

Format the microSD card as **FAT32** and copy the JPEG images to the **root** (no subfolders) using these names:

| Type | File names | Resolution | Count |
|---|---|---|---|
| Landscape | `1o.jpg` … `26o.jpg` | 480x320 | 26 |
| Portrait | `1v.jpg` … `28v.jpg` | 240x320 | 28 |

To change the number of images, edit the `NUM_ORIZZ` and `NUM_VERT` constants in the sketch.

Tips:
- use **baseline** JPEGs (not progressive);
- keep files small: each image is fully loaded into RAM before being decoded (file size and free RAM are printed on the serial monitor).

## Building and uploading

1. Clone the repository and open `main.ino` in the Arduino IDE.
2. Install the libraries and the ESP32 core 3.x listed above.
3. Copy `User_Setup.h` as described in the display section.
4. Select the **ESP32 Dev Module** board and the correct port.
5. Upload the sketch. Open the serial monitor at **9600 baud** to see the logs.

## Customization

| What | Where in the sketch |
|---|---|
| Activation distance (65 cm) | `gestisciDisplay()`: `distanza < 65` |
| Turn-off delay (10 s) | `timeout` constant |
| Displayed phrases | `frasi[]` array |
| Text size and color | `disegnaOverlay()` |
| Component pins | constants at the top of the file |

## Troubleshooting

- **White or black screen:** check `User_Setup.h` (display driver and pins) and the backlight wiring.
- **`ERRORE: SD non disponibile`:** check wiring, FAT32 formatting, and that the SD pins match the ones above.
- **`ERRORE: impossibile aprire ...`:** the file is missing or named differently from what the sketch expects.
- **`ERRORE: memoria insufficiente`:** the image is too large; reduce its size or quality.
- **`ERRORE: decodifica JPEG fallita`:** re-save the file as a baseline JPEG.
- **Odd or always-high distances:** check the sensor's power supply and the voltage divider on the ECHO pin.
- **No phrase or temperature on screen:** the DHT11 is not responding; check the wiring on GPIO 32.

> The serial log messages in the sketch are in Italian.

## License

**TODO:** choose a license (for example MIT) and add a `LICENSE` file.
