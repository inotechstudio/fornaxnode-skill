# Troubleshooting

Always start here: add `Serial.begin(115200); Fornax.debug(Serial);` before `Fornax.begin(...)`, open the Serial
Monitor at **115200** baud, and read the `[Fornax]` lines. On S2, S3 and C3 boards, first set Tools > USB CDC On
Boot > Enabled, or the monitor stays empty.

## Messages from the library

| `[Fornax] ...` | Meaning and fix |
|---|---|
| `FornaxNode 0.1.1 starting, device dev_...` | `begin` ran. If nothing follows, `Fornax.run()` is probably not called in `loop()`. |
| `waiting for Wi-Fi (check the name and password)` | Wrong SSID or password (both are case-sensitive), a 5 GHz-only or hidden network, or out of range. ESP32 boards only join 2.4 GHz. |
| `Wi-Fi connected, IP ...` | Wi-Fi is fine. The clock is set next, then the cloud connection (a few seconds). |
| `connecting to wss://mqtt.fornaxnode.com:443/` | The clock is set and the secure connection is starting. |
| `connected to the cloud` | All good. The dashboard shows the device online. |
| `the cloud refused the login` | `FORNAX_DEVICE_ID` or `FORNAX_TOKEN` is wrong, or the token was replaced on the dashboard. Copy both again from the device page (make a new token if it was lost). |
| `cannot reach the cloud` | No internet on that network, or the clock could not be set (NTP, UDP port 123, blocked). Try a phone hotspot to compare. |
| `disconnected from the cloud, retrying` | The network dropped. The library reconnects by itself. Do not restart the board in code. |
| `command for F2 ignored: no FORNAX_WRITE(F2) in the sketch` | The dashboard sent to a pin the sketch does not handle. Add the handler, or bind the widget to the right pin. |
| `N command(s) dropped: run() is not called often enough` | `loop()` blocks (a long `delay()`, a waiting loop, a slow sensor read). Use `millis()` timing. |
| `FORNAX_DEVICE_ID is not valid (...)` | The ID has a typo or extra characters. Copy it again. |
| `FORNAX_TOKEN is missing or too long` | Copy the token again. |

## Symptoms

| Symptom | Likely cause |
|---|---|
| Build error "FornaxNode supports ESP32 boards only" | A non-ESP32 board is selected. Pick the right board under Tools > Board > esp32. |
| Build error "needs Arduino-ESP32 core 3.0.0 or newer" | Update "esp32 by Espressif Systems" in the Boards Manager. |
| Build error "Put #define FORNAX_DEVICE_ID ... before #include" | The `#define` lines are missing, or come after the include. |
| `'F300' was not declared` or no matching `write(int, ...)` | Pins are `F0` to `F255` and must be written by name. |
| "Sketch too big" | Tools > Partition Scheme > "Huge APP (3MB No OTA/1MB SPIFFS)". |
| Device online, value never shows | No datastream on that pin, or the wrong type (see `datastreams.md`: `bool` vs `digitalRead`, `int` vs `24.5`, enum spelling). |
| Dashboard says "did not confirm the change" | The handler for that pin is missing, `loop()` blocks, or the board is offline. Check the debug log while pressing the switch. |
| Switch flips but the relay does the opposite | The relay module is active-LOW. Swap `HIGH`/`LOW`. |
| Dashboard shows the wrong state after a reboot | Report the state in `Fornax.onConnect(...)`. |
| Board resets when the relay or pump switches | Power: give the load its own supply, share GND, and use a relay module. |
| `analogRead` always 0 or 4095 with Wi-Fi on | It is an ADC2 pin. Move it to ADC1. |
| Readings are `nan` | Sensor wiring or pull-up, or reading too often (DHT22: at most every 2 s). |
| Goes offline every few minutes | Weak power or a poor USB cable, weak Wi-Fi signal, or code that restarts Wi-Fi. |

## The dashboard side

- A device shows **offline** about 45 seconds after it loses power or Wi-Fi.
- If the token is regenerated on the device page, the old one stops working at once: put the new one in the sketch
  and upload again.
- Values the cloud drops (wrong type, unknown pin) do not show anywhere. Check the datastream list on the device page.
