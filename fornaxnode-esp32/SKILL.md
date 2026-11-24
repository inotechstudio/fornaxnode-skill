---
name: fornaxnode-esp32
description: Write, fix and explain Arduino sketches for ESP32 boards (ESP32, ESP32-S2, ESP32-S3, ESP32-C3) that connect to the FornaxNode IoT cloud with the FornaxNode library (Fornax.begin, Fornax.run, Fornax.write, FORNAX_WRITE, Fornax Pins F0 to F255). Use this whenever someone asks for ESP32 code for FornaxNode, wants to send sensor values to a FornaxNode dashboard, control a relay, LED, pump or servo from the FornaxNode dashboard, or debug a board that will not connect to FornaxNode.
---

# FornaxNode for ESP32

FornaxNode (https://fornaxnode.com) is an IoT cloud with a web dashboard. An ESP32 runs a sketch built on the
**FornaxNode Arduino library**. The board sends values to **datastreams** and receives commands from dashboard
widgets. Each datastream sits on a **Fornax Pin**, `F0` to `F255`. A Fornax Pin is a name, not a GPIO, much like
Blynk's virtual pins.

Follow this file whenever you write or change a FornaxNode sketch. Check the details in `references/`:

- `references/api.md`: every library call and its exact behaviour.
- `references/datastreams.md`: data types, what the cloud accepts, and how the dashboard side must be set up.
- `references/boards-and-wiring.md`: safe GPIOs per board, power, and wiring for common parts.
- `references/troubleshooting.md`: Serial Monitor messages and their fixes.
- `examples/`: complete, working sketches to start from.

## The sketch skeleton (always this shape)

```cpp
#define FORNAX_DEVICE_ID "dev_xxxxxxxxxxxx"     // from the dashboard, shown when the device is created
#define FORNAX_TOKEN     "fn_xxxxxxxxxxxxxxxx"  // keep private; a lost token is replaced on the device page
#include <FornaxNode.h>

const char* WIFI_SSID = "your-wifi-name";      // 2.4 GHz Wi-Fi only
const char* WIFI_PASS = "your-wifi-password";

unsigned long lastSend = 0;

void setup() {
  Serial.begin(115200);
  Fornax.debug(Serial);                // prints each connection step
  Fornax.begin(WIFI_SSID, WIFI_PASS);  // returns at once; the library connects in the background
}

void loop() {
  Fornax.run();                        // every loop, as often as possible

  if (millis() - lastSend >= 5000) {   // send every 5 seconds, without delay()
    lastSend = millis();
    Fornax.write(F0, 24.5);
  }
}
```

## Rules you must follow

1. **Put both `#define` lines before `#include <FornaxNode.h>`.** If they are missing, the build stops with an error.
   Use the placeholders `dev_xxxxxxxxxxxx` and `fn_xxxxxxxxxxxxxxxx`. Never invent real-looking values.
2. **Never ask for the user's token or Wi-Fi password, and never repeat them.** If a user pastes a real token, tell
   them to make a new one on the device page and to use placeholders from now on.
3. **Call `Fornax.run()` in every pass of `loop()`, and never block.** Do not use `delay()` longer than a few
   milliseconds, and do not write `while` loops that wait for something. Time things with `millis()`.
   `run()` is what starts the cloud connection, retries Wi-Fi and runs your `FORNAX_WRITE` handlers. While
   `loop()` is blocked, commands wait in a queue of 6, and any more are dropped. The dashboard gives up on a command
   that is not confirmed within about 6 seconds and shows "did not confirm".
4. **Do not touch Wi-Fi or the network.** `Fornax.begin()` connects to Wi-Fi, keeps the connection alive and
   reconnects. Never call `WiFi.begin()`, `WiFi.disconnect()`, `WiFi.mode()`, use an MQTT client or HTTP for the
   cloud, or restart the board to recover from a lost connection.
5. **Use Fornax Pins by name: `F0` to `F255`.** `Fornax.write(1, x)` and `F300` do not compile, which is on purpose.
   A Fornax Pin is not a GPIO: `F2` and GPIO 2 are unrelated.
6. **Each pin has to match a datastream on the dashboard.** The pin and the type must agree (see the table below).
   If a value goes to a pin with no datastream, or has the wrong type, the cloud drops it silently.
7. **Receive commands with `FORNAX_WRITE(Fx)`.** Write it at file level (not inside a function), with a literal pin
   name, and only once per pin. The datastream must have "Dashboard can send commands" ticked.
8. **Report the real state of every control in `Fornax.onConnect(...)`.** This puts the dashboard right after a
   restart or reconnect. You do not need to send the value back inside the handler, because the library confirms
   each command to the cloud with the value it applied.
9. **Send at a sensible rate.** Every 2 to 60 seconds is normal for sensors. Send a control's state when it
   changes, not on every loop. Never send faster than about once a second per pin.
10. **Use only the calls listed in `references/api.md`.** Do not invent functions such as `Fornax.read()`,
    `Fornax.sync()`, `Fornax.notify()`, `Fornax.virtualWrite()` or `BLYNK_*`. If the user wants something the
    library does not do, say so plainly.
11. **Tell the user you have not compiled or run the code.** Say what to test on the bench, and which datastreams
    (pin, type, writable) to create on the dashboard first.

## API at a glance

| Call | Use |
|---|---|
| `Fornax.begin(ssid, pass)` | In `setup()`. Starts Wi-Fi and the cloud connection, then returns at once. |
| `Fornax.run()` | In every `loop()`. Keeps the connection alive and runs `FORNAX_WRITE` handlers. |
| `Fornax.write(Fx, value)` | Sends a value: an integer, a `float`/`double`, a `bool`, text (`const char*` or `String`, up to 256 bytes). Returns `false` if nothing was sent. |
| `Fornax.write(Fx, lat, lon)` | Sends a position to a `location` datastream. |
| `FORNAX_WRITE(Fx) { ... }` | Runs when the dashboard sends a command to `Fx`. Read the value with `param.asBool()`, `asInt()`, `asLong()`, `asFloat()`, `asDouble()`, `asStr()` or `asString()`. |
| `Fornax.onConnect(fn)` / `Fornax.onDisconnect(fn)` | Plain `void fn()` functions, called from `run()` when the cloud connection starts or drops. |
| `Fornax.connected()` / `Fornax.wifiConnected()` | `true` while connected. `write` drops values while offline. |
| `Fornax.debug(Serial)` | Prints progress and errors. Call `Serial.begin(115200)` first. |
| `Fornax.end()` | Tells the cloud the device is going offline, then disconnects. |

## Datastream types and the matching `write`

| Dashboard type | Send | Received in `FORNAX_WRITE` with |
|---|---|---|
| `int` | `Fornax.write(F0, 42);`: whole numbers only. `42.5` is dropped. | `param.asInt()` / `asLong()` |
| `double` | `Fornax.write(F1, 24.5);`: any finite number. `NAN` is not sent. | `param.asDouble()` / `asFloat()` |
| `bool` | `Fornax.write(F2, true);` | `param.asBool()` |
| `string` | `Fornax.write(F3, "hello");` | `param.asString()` |
| `enum` | `Fornax.write(F4, "auto");`: must be exactly one of the options set on the dashboard. | `param.asStr()` |
| `location` | `Fornax.write(F5, 3.139003, 101.686855);` | (not a control) |

Common dashboard widgets: a **switch** or **push button** uses a writable `bool`, a **slider** uses a writable
`int` or `double` with a min and max, a **menu** uses a writable `enum`, and **value**, **gauge**, **line chart**,
**LED** (`bool`) and **location** only display a datastream.

## Patterns

**A control (relay, LED, pump).** Keep its state in a global, apply it in the handler, and report it on connect:

```cpp
const int RELAY_PIN = 26;   // GPIO, see references/boards-and-wiring.md
bool relayOn = false;

FORNAX_WRITE(F0) {                      // writable bool datastream on F0
  relayOn = param.asBool();
  digitalWrite(RELAY_PIN, relayOn ? HIGH : LOW);   // many relay boards switch on with LOW: swap if reversed
}

void reportState() { Fornax.write(F0, relayOn); }

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  Fornax.onConnect(reportState);
  Fornax.begin(WIFI_SSID, WIFI_PASS);
}
```

**A sensor.** Read on a `millis()` timer, skip bad readings, then send:

```cpp
if (millis() - lastSend >= 5000) {
  lastSend = millis();
  float t = dht.readTemperature();
  if (!isnan(t)) Fornax.write(F1, t);
}
```

**A local action plus the cloud.** If the board changes something itself (a button press, a thermostat), apply it,
then send the new state once with `Fornax.write` so the dashboard follows.

## Before you answer, check

- Both `#define` lines come before the include, with placeholders only.
- `Fornax.begin` is in `setup()`, `Fornax.run()` is in every `loop()`, and there is no `delay()` over a few ms and no
  waiting loop.
- Every `Fx` used has a datastream of the right type. List them for the user, for example "F0 bool, writable
  (Relay)".
- Each `FORNAX_WRITE` is at file level, used once per pin, and fast (no waiting inside it).
- The GPIOs suit the board (ADC1 for analog while Wi-Fi is on, no flash pins, no input-only pins for outputs).
- Any extra library (for example "DHT sensor library" by Adafruit) is named, with how to install it.
- The board choice in the IDE is named: Tools > Board > esp32 > ESP32 Dev Module / ESP32S3 / ESP32C3 / ESP32S2 Dev
  Module, with esp32 core 3.0 or newer.

## Install (tell users who do not have it yet)

1. Arduino IDE: Tools > Board > Boards Manager, install **esp32 by Espressif Systems**, version 3.0.0 or newer.
2. Download the library ZIP from https://fornaxnode.com/downloads/FornaxNode.zip, then Sketch > Include Library >
   Add .ZIP Library.
3. On https://app.fornaxnode.com, create a project and a device, add the datastreams, and copy the device ID and
   token into the sketch.

No other libraries are needed for the connection itself.
