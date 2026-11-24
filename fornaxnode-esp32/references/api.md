# FornaxNode library API (version 0.1.1)

These are all the public calls. Anything not listed here does not exist, so do not use it.

Supported boards are ESP32, ESP32-S2, ESP32-S3 and ESP32-C3, with the Arduino-ESP32 core **3.0.0 or newer**. On an
older core, or a board that is not an ESP32, the build stops with a clear error. The library needs no other
libraries.

## Before the include

```cpp
#define FORNAX_DEVICE_ID "dev_xxxxxxxxxxxx"   // required: 4 to 40 characters, letters, digits, - or _
#define FORNAX_TOKEN     "fn_xxxxxxxxxxxxxxxx" // required
// #define FORNAX_BROKER_URI "wss://mqtt.example.com:443/"  // optional: only for a self-hosted server
#include <FornaxNode.h>
```

If `FORNAX_DEVICE_ID` or `FORNAX_TOKEN` is missing, the build stops with an error that says what to add. The
default broker is `wss://mqtt.fornaxnode.com:443/`.

The library defines one global object, `Fornax`. Do not create another.

## Starting and running

### `void Fornax.begin(const char* ssid, const char* password)`
Call this once in `setup()`. It starts Wi-Fi (station mode, with auto-reconnect) and returns at once. The cloud
connection is made later, from `run()`, after Wi-Fi is up and the clock is set over NTP (TLS needs the date). This
takes a few seconds. If the ID or token is not valid, it prints why (with `debug` on) and does nothing more.

### `void Fornax.beginWith(const char* deviceId, const char* token, const char* brokerUri, const char* ssid, const char* password)`
Works like `begin`, with the ID, token and broker given at run time, for example when they are read from storage.
The two `#define` lines must still be there (placeholders are fine) or the include does not build. In almost every
sketch, `begin` is the right choice.

### `void Fornax.run()`
Call this in every pass of `loop()`. On each call it:
- calls your `onConnect` and `onDisconnect` callbacks when the connection state has changed;
- runs the `FORNAX_WRITE` handlers for commands waiting in the queue (up to 6 wait; any more are dropped, and the
  debug log says "command(s) dropped: run() is not called often enough");
- retries Wi-Fi every 20 seconds while it is down;
- starts the cloud connection once Wi-Fi and the clock are ready.

The connection itself (keepalive, TLS, reconnect) runs in its own FreeRTOS task, so a short block does not drop the
device offline. Commands, however, are only handled inside `run()`.

### `void Fornax.end()`
Publishes "offline" to the cloud, disconnects and frees everything. Use it before a planned power-down. It is
rarely needed.

## State

| Call | Returns |
|---|---|
| `bool Fornax.connected()` | `true` while connected to the cloud. `write` returns `false` and sends nothing while this is `false`. |
| `bool Fornax.wifiConnected()` | `true` while Wi-Fi is connected. |

## Callbacks

```cpp
void Fornax.onConnect(void (*callback)());
void Fornax.onDisconnect(void (*callback)());
```

Each callback is a plain function with no arguments and no captures. It runs inside `run()`, in the `loop()` task,
so it may call `Fornax.write`, `Serial` and `digitalWrite`. `onConnect` runs every time the connection comes back,
which makes it the right place to report the real state of controls. Register callbacks before or after `begin`,
either way.

## Debug output

```cpp
Serial.begin(115200);
Fornax.debug(Serial);   // any Print works
```

Each line starts with `[Fornax]`. See `troubleshooting.md` for what the messages mean.

## Sending values: `Fornax.write`

Every overload returns `bool`: `true` if the value was handed to the network, `false` if nothing was sent (offline,
an empty or invalid value, or text that is too long). Readings are sent at QoS 0 and are not stored on the board:
a value written while offline is lost, which is deliberate, because a stale reading arriving late is worse.

| Overload | Notes |
|---|---|
| `write(FornaxPin pin, <any integer type> value)` | `int`, `long`, `uint8_t`, `unsigned long` and so on. Sent as a whole number. |
| `write(FornaxPin pin, float value)` | 7 significant digits. `NAN` or infinity is not sent. |
| `write(FornaxPin pin, double value)` | 15 significant digits. Note that `24.5` is a `double` literal. |
| `write(FornaxPin pin, bool value)` | `true` / `false`. |
| `write(FornaxPin pin, const char* text)` | Up to 256 bytes (UTF-8). Longer text is not sent. |
| `write(FornaxPin pin, const String& text)` | Same as above. |
| `write(FornaxPin pin, double lat, double lon)` | A position in degrees, for a `location` datastream. |

`pin` must be one of the constants `F0` ... `F255` (type `FornaxPin`). A plain number such as `write(3, x)`, or
`F256` and above, does not compile.

The cloud only stores a value when the device has a datastream on that pin and the value fits its type (see
`datastreams.md`). Otherwise it drops the value silently, and `write` still returns `true`, because the board cannot
know.

## Receiving commands: `FORNAX_WRITE`

```cpp
FORNAX_WRITE(F2) {
  bool on = param.asBool();
  digitalWrite(RELAY_PIN, on ? HIGH : LOW);
}
```

- This is a macro that defines a function. Write it at file level (outside `setup`/`loop`), with a literal pin name
  `F0` ... `F255`. Use it at most once per pin in the whole sketch (a second one is a compile error).
- The handler runs inside `Fornax.run()`, in the `loop()` task, never at the same time as your other code. Keep it
  short. Set a variable or a pin, and do the slow work in `loop()`.
- When the handler returns, the library confirms the command to the cloud with the value it applied. The cloud saves
  that value as the newest reading of the datastream, so the dashboard and alerts follow without another `write`.
- A command for a pin with no `FORNAX_WRITE` is ignored and not confirmed. The debug log says
  "command for F2 ignored: no FORNAX_WRITE(F2) in the sketch".

### `param` (a `FornaxParam&`)

| Method | Returns |
|---|---|
| `param.pin()` | The `FornaxPin` this command is for. |
| `param.asBool()` | `true`/`false`. A number counts as true when it is not 0. Text counts as true when it is `"true"`, `"1"`, `"on"` or `"ON"`. |
| `param.asInt()`, `param.asLong()` | The value as a whole number. |
| `param.asFloat()`, `param.asDouble()` | The value as a number. |
| `param.asStr()` | The text, valid only inside the handler (copy it if you keep it). |
| `param.asString()` | The text as an Arduino `String` (a copy, safe to keep). |
| `param.isNumber()`, `param.isBool()`, `param.isString()` | Which kind of value arrived. |

What each dashboard widget sends:

| Widget | Datastream | Value |
|---|---|---|
| Switch | writable `bool` | `true` / `false` |
| Push button | writable `bool` | `true` while pressed, `false` when released |
| Slider | writable `int` or `double` | A number between the min and max set on the widget, sent once the slider stops moving (rounded for `int`) |
| Menu (dropdown) | writable `enum` | One of the enum options, as text |

## Pin names

`F0` ... `F255` are constants of type `FornaxPin`. If a name ever clashes with something in another library, the
library can be regenerated with another prefix (for example `FX0` ... `FX255`). This is rare: do not do it unless
the compiler reports a clash.

## Things that do not exist

`Fornax.read()`, `Fornax.sync()`/`syncAll()`, `Fornax.notify()`/`email()`/`logEvent()`, `Fornax.virtualWrite()`,
`Fornax.setProperty()`, `Fornax.connect()`, timers such as `FornaxTimer`, `FORNAX_CONNECTED()`, `FORNAX_READ()`,
and any `BLYNK_*` macro.

To get the last value of a control after a restart, keep it on the board. Save it with `Preferences` if it must
survive power loss, apply it in `setup()`, and report it in `onConnect`. For a timer, use `millis()`.

Alerts and Telegram messages are set up in the dashboard (Integrations > Telegram, then Alerts on the device page).
They need no code on the board.
