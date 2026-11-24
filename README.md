# FornaxNode skill for AI assistants

Teach your own AI assistant to write correct ESP32 code for **[FornaxNode](https://fornaxnode.com)**, the IoT cloud
with a phone-friendly dashboard for ESP32, ESP32-S2, ESP32-S3 and ESP32-C3 boards.

FornaxNode has its own built-in helper, FornaxAi, in the app. If you would rather use Claude, ChatGPT, Gemini,
Cursor or another assistant, this skill gives it the same knowledge:

- the FornaxNode Arduino library: `Fornax.begin`, `Fornax.run`, `Fornax.write`, `FORNAX_WRITE`, and the Fornax Pins
  `F0` to `F255`;
- the datastream types the cloud accepts, and how to set up the dashboard side (switch, slider, menu, gauge, chart);
- safe GPIOs for each board, wiring and power for common sensors and relays;
- what each Serial Monitor message means, and how to fix it;
- complete example sketches (sensor, relay, thermostat, every data type).

It also stops the common mistakes: `delay()` in `loop()`, touching Wi-Fi, made-up functions, wrong value types,
and asking you for your device token.

## What is inside

```
fornaxnode-esp32/
├── SKILL.md                      the rules and the quick reference (start here)
├── references/
│   ├── api.md                    every library call, exactly
│   ├── datastreams.md            types, widgets, alerts: the dashboard side
│   ├── boards-and-wiring.md      boards, safe GPIOs, power, common parts
│   └── troubleshooting.md        Serial Monitor messages and fixes
└── examples/
    ├── dht22-temperature-humidity.ino
    ├── relay-switch.ino
    ├── thermostat.ino
    └── all-data-types.ino
fornaxnode-esp32.zip              the same folder, zipped, ready to upload
```

## Install

### Claude Code

Copy the `fornaxnode-esp32` folder into your skills folder:

- for all your projects: `~/.claude/skills/fornaxnode-esp32/` (on Windows `C:\Users\<you>\.claude\skills\fornaxnode-esp32\`);
- for one project only: `.claude/skills/fornaxnode-esp32/` inside that project.

```bash
git clone https://github.com/inotechstudio/fornaxnode-skill.git
cp -r fornaxnode-skill/fornaxnode-esp32 ~/.claude/skills/
```

Claude uses it on its own when you ask for FornaxNode or ESP32 code. You can also ask it directly: "use the
fornaxnode-esp32 skill".

### Claude (web and desktop app)

Download `fornaxnode-esp32.zip` from this repository. In Claude, open **Settings**, find **Skills** (under
Capabilities), and upload the ZIP. Skills must be turned on for your account.

### Other assistants (ChatGPT, Gemini, Copilot, Cursor and others)

These do not read skill folders, but you can still give them the knowledge:

- **ChatGPT, Gemini:** create a Project or Gem (or a custom GPT) and add `SKILL.md` and the files in `references/`
  as knowledge files. Or paste the contents of `SKILL.md` into its instructions.
- **Cursor, Windsurf, Copilot:** put `SKILL.md` into your rules file (`.cursor/rules/`, `.windsurfrules`,
  `.github/copilot-instructions.md`), and keep the `references/` folder in the project so the assistant can open
  it.
- **Any chat:** attach `SKILL.md` at the start of the conversation.

## What you still need

1. An account on [app.fornaxnode.com](https://app.fornaxnode.com), with a device and its datastreams.
2. The Arduino IDE with **esp32 by Espressif Systems 3.0.0 or newer** (Boards Manager).
3. The FornaxNode library: [download the ZIP](https://fornaxnode.com/downloads/FornaxNode.zip), then Sketch >
   Include Library > Add .ZIP Library.

## Keep your token private

Your device token works like a password for the device. Never paste it into an AI chat. Ask for code with the
placeholders `dev_xxxxxxxxxxxx` and `fn_xxxxxxxxxxxxxxxx`, then put your real ID and token in the sketch yourself. If
a token is ever shared, make a new one on the device page.

## AI code is not tested code

An assistant using this skill writes code that follows the library correctly, but nobody has compiled it or run it
on your board. Test it on your bench before you rely on it, above all anything that switches mains power, heaters
or pumps.

## Updating

The skill matches FornaxNode library **0.1.1**. When the library changes, update this repository: copy the folder
again (Claude Code) or upload the new ZIP (Claude).

## License

MIT. See [LICENSE](LICENSE).
