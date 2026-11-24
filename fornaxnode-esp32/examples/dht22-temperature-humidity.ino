// FornaxNode example: a DHT22 sends temperature and humidity every 5 seconds.
//
// Board: any ESP32 (pick it under Tools > Board > esp32). esp32 core 3.0 or newer.
// Libraries: FornaxNode (https://fornaxnode.com/downloads/FornaxNode.zip) and
//            "DHT sensor library" by Adafruit (Library Manager).
// Wiring: DHT22 VCC to 3.3V, GND to GND, DATA to GPIO 4 (bare sensors also need 10k from DATA to 3.3V).
//
// Datastreams to create on the device page:
//   F0  Temperature  double  unit °C
//   F1  Humidity     double  unit %

#define FORNAX_DEVICE_ID "dev_xxxxxxxxxxxx"
#define FORNAX_TOKEN     "fn_xxxxxxxxxxxxxxxx"
#include <FornaxNode.h>
#include <DHT.h>

const char* WIFI_SSID = "your-wifi-name";      // 2.4 GHz Wi-Fi only
const char* WIFI_PASS = "your-wifi-password";

const int DHT_PIN = 4;
DHT dht(DHT_PIN, DHT22);

unsigned long lastSend = 0;

void setup() {
  Serial.begin(115200);
  dht.begin();
  Fornax.debug(Serial);
  Fornax.begin(WIFI_SSID, WIFI_PASS);
}

void loop() {
  Fornax.run();

  if (millis() - lastSend >= 5000) {   // the DHT22 needs at least 2 s between reads
    lastSend = millis();
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("The DHT22 did not answer: check its wiring");
      return;
    }
    Fornax.write(F0, temperature);
    Fornax.write(F1, humidity);
  }
}
