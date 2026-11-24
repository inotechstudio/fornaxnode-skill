# Datastreams: the dashboard side

A sketch only works when the device on the dashboard has a matching datastream for every Fornax Pin it uses. When you
give someone code, also list the datastreams to create, for example:

> On the device page, add these datastreams:
> - `F0`, Temperature, type `double`, unit `°C`
> - `F1`, Relay, type `bool`, **Dashboard can send commands** ticked

## Fields of a datastream

| Field | Meaning |
|---|---|
| Pin | `F0` to `F255`. The pin identifies the datastream. If left empty, the next free pin is used. |
| Label | A name shown on the dashboard, which can be changed later. |
| Type | `int`, `double`, `bool`, `string`, `enum` or `location`. Pick it carefully: the cloud checks every value against it. |
| Unit | Optional text shown after the value (°C, %, V). |
| Options | For `enum` only: the allowed values, separated by commas (for example `auto, manual, off`). |
| Dashboard can send commands | Makes it a control. Without this, the dashboard cannot send to the pin, and `FORNAX_WRITE` never runs. |

## What the cloud accepts per type

| Type | Accepted | Dropped |
|---|---|---|
| `int` | A whole number (an integer type in C++, or a whole-valued `double`) | `24.5`, text, `true` |
| `double` | Any finite number, whole numbers included | `NAN`, infinity (the library does not send these), text |
| `bool` | `true` / `false` (`Fornax.write(F0, true)`) | `1`/`0` sent as a number: use a `bool` variable or `x != 0` |
| `string` | Text up to 256 bytes | Numbers. Convert with `String(x)` if you really want text |
| `enum` | Exactly one of the options, matching case | Anything else, `"Auto"` when the option is `auto` |
| `location` | `Fornax.write(F0, lat, lon)` with lat -90..90 and lon -180..180 | Out-of-range or `NAN` coordinates |

A dropped value gives no error on the board: `write` still returns `true`. If a value never shows on the dashboard,
first check that the pin and type match.

Watch out for `bool`. `Fornax.write(F0, digitalRead(PIN))` sends an **int** (digitalRead returns `int`), so a `bool`
datastream drops it. Write `Fornax.write(F0, digitalRead(PIN) == HIGH)` instead.

Likewise, `Fornax.write(F0, analogRead(PIN))` sends an int: right for an `int` datastream, but it also fits a `double`
one.

## Widgets and the datastreams they need

| Widget | Datastream | Direction |
|---|---|---|
| Value | any type except `location` | shows the latest value |
| Gauge | `int` or `double` | shows the latest value on a dial (min and max set on the widget) |
| Line chart | `int`, `double` or `bool` | shows history (1 hour, 24 hours, 7 days) |
| LED | `bool` | lit while the value is `true` |
| Location | `location` | shows the position with a map link |
| Switch | writable `bool` | sends `true`/`false` |
| Push button | writable `bool` | sends `true` on press and `false` on release |
| Slider | writable `int` or `double` | sends a number between the widget's min and max |
| Menu | writable `enum` | sends one of the options |

A writable datastream is both a control and a display. The board receives commands on it, and the value the board
confirms (or writes) is what the dashboard shows.

## Alerts and Telegram (no code on the board)

Alerts live in the dashboard, not in the sketch. In Integrations > Telegram, the user links a Telegram bot. Then, on
a device page under Alerts, they can add rules:

- the device goes offline, or comes back online;
- a value crosses a limit (`>`, `>=`, `<`, `<=`, `==`, `!=`), with a cooldown and an optional custom message;
- a report of the latest values, every N minutes or daily.

So for "send me a message when the temperature is above 30", write a sketch that just sends the temperature, and
tell the user to add a value alert. Do not write Telegram, HTTP or e-mail code on the board.

## Timestamps

The cloud stamps each value with its arrival time. The library does not send its own timestamps, and you do not
need to.
