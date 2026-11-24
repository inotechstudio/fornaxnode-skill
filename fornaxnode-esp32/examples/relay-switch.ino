// FornaxNode example: switch a relay (or LED) on and off from the dashboard.
//
// Board: any ESP32. esp32 core 3.0 or newer. Library: FornaxNode.
// Wiring: relay module VCC, GND, IN to GPIO 26 (on S2/S3/C3 boards use GPIO 5).
//
// Datastream to create on the device page:
//   F0  Relay  bool  "Dashboard can send commands" ticked
// Then add a Switch widget bound to F0 on the project dashboard.

#define FORNAX_DEVICE_ID "dev_xxxxxxxxxxxx"
#define FORNAX_TOKEN     "fn_xxxxxxxxxxxxxxxx"
#include <FornaxNode.h>

const char* WIFI_SSID = "your-wifi-name";      // 2.4 GHz Wi-Fi only
const char* WIFI_PASS = "your-wifi-password";

const int RELAY_PIN = 26;
// Many relay modules switch on with LOW. If yours works the wrong way round, swap ON_LEVEL and OFF_LEVEL.
const int ON_LEVEL = HIGH;
const int OFF_LEVEL = LOW;

bool relayOn = false;

// Runs when the Switch on the dashboard is flipped. The library confirms the new value to the cloud after it returns.
FORNAX_WRITE(F0) {
  relayOn = param.asBool();
  digitalWrite(RELAY_PIN, relayOn ? ON_LEVEL : OFF_LEVEL);
  Serial.printf("relay %s\n", relayOn ? "on" : "off");
}

// Each time the board (re)connects, tell the dashboard the real state.
void reportState() {
  Fornax.write(F0, relayOn);
}

void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, OFF_LEVEL);
  Fornax.debug(Serial);
  Fornax.onConnect(reportState);
  Fornax.begin(WIFI_SSID, WIFI_PASS);
}

void loop() {
  Fornax.run();
}
