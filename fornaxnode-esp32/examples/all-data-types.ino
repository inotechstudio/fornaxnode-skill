// FornaxNode example: one write for every datastream type, and a command from a Menu.
//
// Board: any ESP32. esp32 core 3.0 or newer. Library: FornaxNode.
//
// Datastreams to create on the device page:
//   F0  Counter   int
//   F1  Pi        double
//   F2  Even      bool
//   F3  Message   string
//   F4  Mode      enum      options: auto, manual, off    "Dashboard can send commands" ticked
//   F5  Position  location

#define FORNAX_DEVICE_ID "dev_xxxxxxxxxxxx"
#define FORNAX_TOKEN     "fn_xxxxxxxxxxxxxxxx"
#include <FornaxNode.h>

const char* WIFI_SSID = "your-wifi-name";      // 2.4 GHz Wi-Fi only
const char* WIFI_PASS = "your-wifi-password";

unsigned long lastSend = 0;
int counter = 0;
String mode = "auto";

// A writable enum arrives as text: one of the options defined on the dashboard.
FORNAX_WRITE(F4) {
  mode = param.asString();          // asString() copies; asStr() is only valid inside the handler
  Serial.printf("mode is now %s\n", mode.c_str());
}

void setup() {
  Serial.begin(115200);
  Fornax.debug(Serial);
  Fornax.onConnect([] { Fornax.write(F4, mode); });   // a lambda with no captures works as a callback
  Fornax.begin(WIFI_SSID, WIFI_PASS);
}

void loop() {
  Fornax.run();

  if (millis() - lastSend >= 10000) {
    lastSend = millis();
    Fornax.write(F0, ++counter);                     // int
    Fornax.write(F1, 3.14159);                       // double
    Fornax.write(F2, (counter % 2) == 0);            // bool (a comparison is a bool)
    Fornax.write(F3, "hello from the ESP32");        // string, up to 256 bytes
    Fornax.write(F5, 3.139003, 101.686855);          // location: latitude, longitude
  }
}
