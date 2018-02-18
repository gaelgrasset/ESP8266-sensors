#include <Homie.h>
#include <WEMOS_SHT3X.h>

#define CUSTOM_FIRMWARE_NAME "sht30shield-d1mini-sensor"
#define CUSTOM_FIRMWARE_VERSION "1.0.1"

SHT3X sht30(0x45); // SHT 30 address - Default on D1 mini shield

HomieSetting<long> sensorReadingInterval("interval", "Sensor reading interval");

long lastTemperatureSent = 0;


HomieNode temperatureNode("temperature", "temperature");
HomieNode humidityNode("humidity", "humidity");

void setupHandler() {
  // Units
  temperatureNode.setProperty("unit").send("C");
  humidityNode.setProperty("unit").send("%");

  // Settings
  sensorReadingInterval.setDefaultValue(300).setValidator([] (long candidate) {
    return (candidate >= 0) && (candidate <= 3600);
  });
}

void loopHandler() {
  if (millis() - lastTemperatureSent >= sensorReadingInterval.get() * 1000L || lastTemperatureSent == 0) {
    if (sht30.get() == 0 ) {

      // Send Temperature
      float temperature = sht30.cTemp; // Celsius Temperature
      Homie.getLogger() << "Temperature: " << temperature << " °C" << endl;
      temperatureNode.setProperty("degrees").send(String(temperature));

      // Send Temperature
      float hum = sht30.humidity; // Relative humidity
      Homie.getLogger() << "Humidity: " << hum << " %" << endl;
      humidityNode.setProperty("relative").send(String(hum));

      lastTemperatureSent = millis();
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial << endl << endl;
  Homie_setFirmware(CUSTOM_FIRMWARE_NAME, CUSTOM_FIRMWARE_VERSION);
  Homie.setSetupFunction(setupHandler).setLoopFunction(loopHandler);
  Homie.setup();
}

void loop() {
  Homie.loop();
}