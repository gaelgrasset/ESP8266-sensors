#define FASTLED_ESP8266_NODEMCU_PIN_ORDER
#include <FastLED.h>
#include <Homie.h>

// FASTLED Definitions
#define LED_PIN     5
#define NUM_LEDS    50
#define BRIGHTNESS  64
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB

// HOMIE Definitions
#define CUSTOM_FIRMWARE_NAME "NODEMCU-WS1821B-CONTROL"
#define CUSTOM_FIRMWARE_VERSION "1.0.0"

CRGB leds[NUM_LEDS];

HomieSetting<long> ledNumber("ledNumber", "Number of leds on the attached strip");

HomieNode ledStrip("strip", "strip");

bool ledColorHandler(const HomieRange& range, const String& value) {
  if (value == "red") {
    for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB::Red;        
    }
  }
  ledStrip.setProperty("color").send(value);
  Homie.getLogger() << "Led color is " << value << endl;

  return true;
}

bool ledBrightnessHandler(const HomieRange& range, const String& value) {
  return true;
}

void setupHandler() {

  // Validate Settings
  ledNumber.setDefaultValue(100).setValidator([] (long candidate) {
    return (candidate >= 0) && (candidate <= 500);
  });  

  // Setup properties and handlers
  ledStrip.advertise("color").settable(ledColorHandler);
  ledStrip.advertise("brightness").settable(ledBrightnessHandler);

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );
}

void setup() {
  Serial.begin(115200);
  Serial << endl << endl;
  Homie_setFirmware(CUSTOM_FIRMWARE_NAME, CUSTOM_FIRMWARE_VERSION);
  Homie.setSetupFunction(setupHandler);

  // Increase mqtt client Keep Alive. To avoid broker disconnection issues.
  // Let's put it to 24h
  Homie.getLogger() << "Force MQTT Connection keep alive at 24h (86400 seconds)" << endl;
  Homie.getMqttClient().setKeepAlive(86400);
  
  Homie.setup();
}

void loop() {
  Homie.loop();
}
