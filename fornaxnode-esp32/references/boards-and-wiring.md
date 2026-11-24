# Boards, GPIOs and wiring

## Boards and what to pick in the Arduino IDE

All boards need **esp32 by Espressif Systems 3.0.0 or newer** (Boards Manager).

| Board | Tools > Board > esp32 > | Good default GPIOs (output / data / analog) | Watch out |
|---|---|---|---|
| ESP32 (DevKit, WROOM-32, NodeMCU-32S) | ESP32 Dev Module | 26 / 4 / 34 | USB driver (CP2102 or CH340). Hold BOOT if the upload stops at "Connecting...". |
| ESP32-S3 (DevKitC-1, S3 Zero) | ESP32S3 Dev Module | 5 / 4 / 1 | On the native "USB" port set Tools > USB CDC On Boot > Enabled, or Serial stays empty. |
| ESP32-C3 (Super Mini, DevKitM-1) | ESP32C3 Dev Module | 5 / 4 / 1 | Set USB CDC On Boot > Enabled. Little flash: choose Partition Scheme "Huge APP". |
| ESP32-S2 (S2 Mini, Saola) | ESP32S2 Dev Module | 5 / 4 / 1 | Set USB CDC On Boot > Enabled. First upload: hold BOOT, tap RESET, release BOOT, then pick the new port. |

If an upload says the sketch is too big, choose Tools > Partition Scheme > "Huge APP (3MB No OTA/1MB SPIFFS)". The
secure connection makes FornaxNode sketches larger than a blink sketch.

## GPIO rules

- **Analog inputs must use ADC1 while Wi-Fi is on.** ADC2 does not work then.
  - ESP32: ADC1 is GPIO 32-39 (use 32, 33, 34, 35, 36 or 39). Never use ADC2 (GPIO 0, 2, 4, 12-15, 25-27) for
    `analogRead`.
  - ESP32-S2 and S3: ADC1 is GPIO 1-10.
  - ESP32-C3: ADC1 is GPIO 0-4.
- **ESP32 GPIO 34, 35, 36 and 39 are input only.** They have no internal pull-up and cannot drive a relay or LED.
- **Do not use flash pins.** On the ESP32 these are GPIO 6-11. On the S2 and S3, GPIO 26-32 are used by the flash
  (and 33-37 on boards with octal PSRAM).
- **Avoid strapping pins for things that hold a level at boot**: ESP32 GPIO 0, 2, 5, 12, 15; S2/S3 GPIO 0, 3, 45,
  46; C3 GPIO 2, 8, 9. A relay or sensor pulling them can stop the board from booting.
- **3.3 V logic.** GPIOs take at most 3.3 V. A sensor that outputs 5 V needs a voltage divider (for example 10 kΩ and
  20 kΩ) or a level shifter on its output.
- **I2C defaults** (`Wire.begin()` with no pins): ESP32 SDA 21 / SCL 22; S3 SDA 8 / SCL 9; C3 SDA 8 / SCL 9; S2 SDA 8 /
  SCL 9. You can pass other pins with `Wire.begin(SDA, SCL)`. Keep them off the strapping pins above where you can.
- PWM: `analogWrite(pin, 0..255)` works on any output-capable GPIO with core 3.x. For servos use the "ESP32Servo"
  library.

## Power

- Relays, pumps, motors and LED strips need their own supply. Join its ground to the ESP32 ground.
- Use a relay **module** (with a transistor and diode), not a bare relay coil on a GPIO.
- A motor or pump on the same weak USB supply causes resets and Wi-Fi drops. Use a separate supply.
- Many relay modules switch **on with LOW**. If the relay works the wrong way round, swap `HIGH` and `LOW` in the
  handler and in `setup()`.

## Common parts

| Part | Wiring | Library and code |
|---|---|---|
| DHT22 / AM2302 (temperature, humidity) | VCC 3.3 V, GND, DATA to a GPIO (default 4), 10 kΩ pull-up DATA-VCC if it is a bare sensor | "DHT sensor library" (Adafruit): `DHT dht(4, DHT22); dht.begin(); dht.readTemperature(); dht.readHumidity();`. Read at most every 2 s and skip `isnan` values. |
| DS18B20 (waterproof temperature) | VCC 3.3 V, GND, DATA to a GPIO with a 4.7 kΩ pull-up to 3.3 V | "OneWire" + "DallasTemperature": `sensors.requestTemperatures(); sensors.getTempCByIndex(0)`. Skip `-127` (disconnected) and `85` (not ready). |
| Capacitive soil moisture | VCC 3.3 V, GND, AOUT to an ADC1 pin | `analogRead`. Calibrate dry and wet readings, then map to 0-100 %. |
| BME280 (temperature, humidity, pressure) | I2C: SDA, SCL, 3.3 V, GND | "Adafruit BME280": `bme.begin(0x76)` (or `0x77`). |
| BH1750 (light, lux) | I2C | "BH1750" (Christopher Laws): `lightMeter.begin(); lightMeter.readLightLevel()`. |
| HC-SR04 (distance) | VCC 5 V, TRIG to a GPIO, ECHO through a divider to a GPIO (5 V output) | `pulseIn(ECHO, HIGH, 30000)`, then cm = µs / 58. The timeout keeps it from blocking. |
| PIR motion (HC-SR501) | VCC 5 V, OUT to a GPIO (3.3 V output) | `digitalRead(PIN) == HIGH`. Send it only when it changes. |
| Relay module | VCC, GND, IN to a GPIO | `digitalWrite`. See the active-LOW note above. |
| LED with resistor, MOSFET for a strip | GPIO > 220 Ω > LED > GND, or GPIO > MOSFET gate | `analogWrite(pin, 0..255)` for brightness. |
| Servo | Signal to a GPIO, power from 5 V (not the ESP32 pin), shared GND | "ESP32Servo": `servo.attach(pin); servo.write(angle);` |
| MQ-2 / MQ-135 gas | Heater needs 5 V. AOUT is up to 5 V, so use a divider to an ADC1 pin | `analogRead`. Treat it as a relative level. Needs a warm-up. |
| 16x2 LCD with I2C backpack | I2C, 5 V | "LiquidCrystal I2C": address usually `0x27`. |
| SSD1306 OLED 128x64 | I2C, 3.3 V | "Adafruit SSD1306" + "Adafruit GFX": `display.begin(SSD1306_SWITCHCAPVCC, 0x3C)`. |

Name each extra library exactly as it appears in Sketch > Include Library > Manage Libraries.
