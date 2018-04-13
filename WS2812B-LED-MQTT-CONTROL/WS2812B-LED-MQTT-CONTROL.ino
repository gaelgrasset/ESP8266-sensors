#define FASTLED_ESP8266_NODEMCU_PIN_ORDER
#include <FastLED.h>
#include <Homie.h>

// FASTLED Definitions
#define LED_PIN     5
#define NUM_LEDS    120
#define BRIGHTNESS  64
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define UPDATES_PER_SECOND 100

// HOMIE Definitions
#define CUSTOM_FIRMWARE_NAME "NODEMCU-WS1821B-CONTROL"
#define CUSTOM_FIRMWARE_VERSION "1.0.0"

CRGB leds[NUM_LEDS];

HomieSetting<long> ledNumber("ledNumber", "Number of leds on the attached strip");

HomieNode ledNode("led", "led");

// RGB Values given by openHab
int SoffitR;
int SoffitG;
int SoffitB;

bool ledColorHandler(const HomieRange& range, const String& value) {
  // Off the leds
  if (value == "off") {
    Homie.getLogger() << "All black" << value << endl;
    
    setLedUniqueColor(0, 0, 0);    // Black/off
    ledNode.setProperty("color").send("0,0,0");
    ledNode.setProperty("brightness").send("0");

  } else {
    Homie.getLogger() << "Led color : " << value << endl;
    
    // split string at every "," and store in proper variable
    // convert final result to integer
    
    SoffitR = value.substring(0, value.indexOf(',')).toInt();
    SoffitG = value.substring(value.indexOf(',') + 1, value.lastIndexOf(',')).toInt();
    SoffitB = value.substring(value.lastIndexOf(',') + 1).toInt();
    Homie.getLogger() << "Red : " << SoffitR << " - Green : " << SoffitG << " - Blue : " << SoffitB << endl;
    
    setLedUniqueColor(SoffitR, SoffitG, SoffitB);
    
    ledNode.setProperty("color").send(value);
  }
  return true;
}

void setLedUniqueColor(uint8_t r, uint8_t g, uint8_t b) {
   // Step 1 : Fill all led strip with given color.
   // TODO : apply cool transition
   fill_solid(leds, NUM_LEDS, CRGB(r,g,b));
   FastLED.show(); 
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
  ledNode.advertise("color").settable(ledColorHandler);
  ledNode.advertise("brightness").settable(ledBrightnessHandler);

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(BRIGHTNESS);
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
