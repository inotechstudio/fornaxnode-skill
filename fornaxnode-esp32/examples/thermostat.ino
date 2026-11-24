// FornaxNode example: a thermostat. A DHT22 measures the room, a relay drives a heater (or a fan),
// and the dashboard sets the mode and the target temperature.
//
// Board: any ESP32. esp32 core 3.0 or newer.
// Libraries: FornaxNode and "DHT sensor library" by Adafruit.
// Wiring: DHT22 DATA to GPIO 4, relay module IN to GPIO 26 (on S2/S3/C3 boards use GPIO 5 for the relay).
//
// Datastreams to create on the device page:
//   F0  Temperature  double  unit °C
//   F1  Heater       bool    writable     (Switch widget: switching it by hand turns Mode to manual)
//   F2  Mode         enum    writable     options: auto, manual   (Menu widget)
//   F3  Target       double  writable     unit °C                 (Slider widget, min 10, max 35)

#define FORNAX_DEVICE_ID "dev_xxxxxxxxxxxx"
#define FORNAX_TOKEN     "fn_xxxxxxxxxxxxxxxx"
#include <FornaxNode.h>
#include <DHT.h>

const char* WIFI_SSID = "your-wifi-name";      // 2.4 GHz Wi-Fi only
const char* WIFI_PASS = "your-wifi-password";

const int DHT_PIN = 4;
const int HEATER_PIN = 26;
const float HYSTERESIS = 0.5;   // °C either side of the target, so the relay does not chatter

DHT dht(DHT_PIN, DHT22);

bool heaterOn = false;
bool autoMode = true;
double target = 22.0;
unsigned long lastRead = 0;

void applyHeater(bool on) {
  heaterOn = on;
  digitalWrite(HEATER_PIN, on ? HIGH : LOW);
}

// The Switch: a manual choice, so the thermostat stops deciding by itself.
FORNAX_WRITE(F1) {
  applyHeater(param.asBool());
  if (autoMode) {
    autoMode = false;
    Fornax.write(F2, "manual");   // let the Menu widget follow
  }
}

// The Menu: "auto" or "manual".
FORNAX_WRITE(F2) {
  autoMode = strcmp(param.asStr(), "auto") == 0;
}

// The Slider: the target temperature.
FORNAX_WRITE(F3) {
  target = constrain(param.asDouble(), 5.0, 40.0);
}

void reportState() {
  Fornax.write(F1, heaterOn);
  Fornax.write(F2, autoMode ? "auto" : "manual");
  Fornax.write(F3, target);
}

void setup() {
  Serial.begin(115200);
  pinMode(HEATER_PIN, OUTPUT);
  applyHeater(false);
  dht.begin();
  Fornax.debug(Serial);
  Fornax.onConnect(reportState);
  Fornax.begin(WIFI_SSID, WIFI_PASS);
}

void loop() {
  Fornax.run();

  if (millis() - lastRead < 5000) return;
  lastRead = millis();

  float t = dht.readTemperature();
  if (isnan(t)) {
    Serial.println("The DHT22 did not answer: check its wiring");
    return;
  }
  Fornax.write(F0, t);

  if (autoMode) {
    bool want = heaterOn;
    if (t < target - HYSTERESIS) want = true;
    if (t > target + HYSTERESIS) want = false;
    if (want != heaterOn) {
      applyHeater(want);
      Fornax.write(F1, heaterOn);   // the board changed it, so tell the dashboard
    }
  }
}
