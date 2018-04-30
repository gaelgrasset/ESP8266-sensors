/*
   NODEMCU + FastLED + MQTT
   Adapted from code under GNU License here : https://github.com/jasoncoon/esp8266-fastled-webserver
   Copyright (C) 2015 Jason Coon
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


// TODO Flickering LED's ... https://github.com/FastLED/FastLED/issues/394 or https://github.com/FastLED/FastLED/issues/306
#define FASTLED_ALLOW_INTERRUPTS 0
#define FASTLED_ESP8266_NODEMCU_PIN_ORDER
//#define FASTLED_INTERRUPT_RETRY_COUNT 0
#include "FastLED.h"
FASTLED_USING_NAMESPACE

extern "C" {
#include "user_interface.h"
}

#include <Homie.h>
#include "GradientPalettes.h"
#include "Settings.h"

CRGB leds[NUM_LEDS];

uint8_t patternIndex = 0;

const uint8_t brightnessCount = 5;
uint8_t brightnessMap[brightnessCount] = { 16, 32, 64, 128, 255 };
int brightnessIndex = 0;
uint8_t brightness = brightnessMap[brightnessIndex];

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

// ten seconds per color palette makes a good demo
// 20-120 is better for deployment
#define SECONDS_PER_PALETTE 120

///////////////////////////////////////////////////////////////////////

// Forward declarations of an array of cpt-city gradient palettes, and
// a count of how many there are.  The actual color palette definitions
// are at the bottom of this file.
extern const TProgmemRGBGradientPalettePtr gGradientPalettes[];
extern const uint8_t gGradientPaletteCount;

// Current palette number from the 'playlist' of color palettes
uint8_t gCurrentPaletteNumber = 0;

CRGBPalette16 gCurrentPalette( CRGB::Black);
CRGBPalette16 gTargetPalette( gGradientPalettes[0] );

uint8_t currentPatternIndex = 0; // Index number of which pattern is current
bool autoplayEnabled = false;

uint8_t autoPlayDurationSeconds = 10;
unsigned int autoPlayTimeout = 0;

uint8_t gHue = 0; // rotating "base color" used by many of the patterns

CRGB solidColor = CRGB::Black;

uint8_t power = 1;

typedef void (*Pattern)();
typedef Pattern PatternList[];
typedef struct {
  Pattern pattern;
  String name;
} PatternAndName;
typedef PatternAndName PatternAndNameList[];
// List of patterns to cycle through.  Each is defined as a separate function below.

#include "PatternLogics.h"

PatternAndNameList patterns = {
  { colorwaves, "Color Waves" },
  { palettetest, "Palette Test" },
  { pride, "Pride" },
  { rainbow, "Rainbow" },
  { rainbowWithGlitter, "Rainbow With Glitter" },
  { confetti, "Confetti" },
  { sinelon, "Sinelon" },
  { juggle, "Juggle" },
  { bpm, "BPM" },
  { fire, "Fire" },
  { showSolidColor, "Solid Color" },
};
const uint8_t patternCount = ARRAY_SIZE(patterns);

#include "Inits.h"

HomieNode ledNode("led", "led");

/*
 * Setup of Arduino program
 * Setup Leds, and then call the Homie Setup
 */
void setup() {
  Serial.begin(115200);
  Serial << endl << endl;
  Homie_setFirmware(CUSTOM_FIRMWARE_NAME, CUSTOM_FIRMWARE_VERSION);
  
  delay(100);
  //Serial.setDebugOutput(true);
  loadSettings();
  initFastLED();
  
  // Setup properties and handlers
  ledNode.advertise("command").settable(ledCommandHandler);

  autoPlayTimeout = millis() + (autoPlayDurationSeconds * 1000);

  // Increase mqtt client Keep Alive. To avoid broker disconnection issues.
  // Let's put it to 24h
  Homie.getLogger() << "Force MQTT Connection keep alive at 24h (86400 seconds)" << endl;
  Homie.getMqttClient().setKeepAlive(500);
  Homie.setLoopFunction(loopHandler);

  Homie.setup();
}

/*
 * Handler of MQTT topic message.
 * Format is: command:value
 * value has to be a number, except rgb commands
 */
bool ledCommandHandler (const HomieRange& range, const String& value) { 
 
   String data(value);
   
   Homie.getLogger() << "Received Data from Topic: " << data << endl;
   Homie.getLogger() << endl;
  
   if ( data.length() > 0) {

    if (data.startsWith("rgb(")) {
      data.replace("rgb(","");
      String r =  getValue(data, ',', 0);
      String g =  getValue(data, ',', 1);
      String b =  getValue(data, ',', 2);
      b.replace(")","");

      Homie.getLogger() << "Received R: " << r.c_str() << " G: " << g.c_str() << " B: " << b.c_str() << endl;      
      
      if (r.length() > 0 && g.length() > 0 && b.length() > 0) {
        setSolidColor(r.toInt(), g.toInt(), b.toInt());
      }
    }else {
      String command =  getValue(data, ':', 0);
      String commandValue = getValue(data, ':', 1);
  
      if (command.length() > 0) {
  
        if (command.equals("power")) {
          if (isValidNumber(commandValue)) {
            setPower(commandValue.toInt());
          }
        } else if (command.equals("solidcolor")) {
          String r =  getValue(commandValue, ':', 2);
          String g =  getValue(commandValue, ':', 4);
          String b =  getValue(commandValue, ':', 6);
          
          Homie.getLogger() << "Received R: " << r.c_str() << " G: " << g.c_str() << " B: " << b.c_str() << endl;

          if (r.length() > 0 && g.length() > 0 && b.length() > 0) {
            setSolidColor(r.toInt(), g.toInt(), b.toInt());
          }
        } else if (command.equals("pattern")) {
          if (isValidNumber(commandValue)) {
            setPattern(commandValue.toInt());
          }
        } else if (command.equals("brightness")) {
          if (isValidNumber(commandValue)) {
            setBrightness(commandValue.toInt());
          }
        } else if (command.equals("brightnessAdjust")) {
          if (isValidNumber(commandValue)) {
            adjustBrightness(commandValue.toInt() == 0 ? false : true);
          }
        } else if (command.equals("patternAdjust")) {
          if (isValidNumber(commandValue)) {
            adjustPattern(commandValue.toInt() == 0 ? false : true);
          }
        }
      }
    }
    
   ledNode.setProperty("command").send(data);
  }
  return true;
}

void loop() {
    Homie.loop();
 }   

void loopHandler() {
 
  // Add entropy to random number generator; we use a lot of it.
  random16_add_entropy(random(65535));
 
  if (power == 0) {
    Homie.getLogger() << "Power 0 " << endl;
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    return;
  }

   EVERY_N_MILLISECONDS( 20 ) {
    gHue++;  // slowly cycle the "base color" through the rainbow
  }

  // change to a new cpt-city gradient palette
  EVERY_N_SECONDS( SECONDS_PER_PALETTE ) {
    gCurrentPaletteNumber = addmod8( gCurrentPaletteNumber, 1, gGradientPaletteCount);
    gTargetPalette = gGradientPalettes[ gCurrentPaletteNumber ];
  }

  // slowly blend the current cpt-city gradient palette to the next
  EVERY_N_MILLISECONDS(40) {
    nblendPaletteTowardPalette( gCurrentPalette, gTargetPalette, 16);
  }

  if (autoplayEnabled && millis() > autoPlayTimeout) {
    adjustPattern(true);
    autoPlayTimeout = millis() + (autoPlayDurationSeconds * 1000);
  }

  // Call the current pattern function once, updating the 'leds' array
  patterns[currentPatternIndex].pattern();

  FastLED.show();

  // insert a delay to keep the framerate modest
  delay(1000 / FRAMES_PER_SECOND);
}


void setPower(uint8_t value)
{
  power = value == 0 ? 0 : 1;  
}

void setSolidColor(CRGB color)
{
  setSolidColor(color.r, color.g, color.b);
}

void setSolidColor(uint8_t r, uint8_t g, uint8_t b)
{
  solidColor = CRGB(r, g, b);
  setPattern(patternCount - 1);
}

// increase or decrease the current pattern number, and wrap around at theends
void adjustPattern(bool up)
{
  if (up)
    currentPatternIndex++;
  else
    currentPatternIndex--;

  // wrap around at the ends
  if (currentPatternIndex < 0)
    currentPatternIndex = patternCount - 1;
  if (currentPatternIndex >= patternCount)
    currentPatternIndex = 0;
}

void setPattern(int value)
{
  // don't wrap around at the ends
  if (value < 0)
    value = 0;
  else if (value >= patternCount)
    value = patternCount - 1;

  currentPatternIndex = value;
}

// adjust the brightness, and wrap around at the ends
void adjustBrightness(bool up)
{
  if (up)
    brightnessIndex++;
  else
    brightnessIndex--;

  // wrap around at the ends
  if (brightnessIndex < 0)
    brightnessIndex = brightnessCount - 1;
  else if (brightnessIndex >= brightnessCount)
    brightnessIndex = 0;

  brightness = brightnessMap[brightnessIndex];

  FastLED.setBrightness(brightness);
}

void setBrightness(int value)
{
  // don't wrap around at the ends
  if (value > 255)
    value = 255;
  else if (value < 0) value = 0;

  brightness = value;

  FastLED.setBrightness(brightness);
}

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}


boolean isValidNumber(String str) {
  // TODO replace with regex check
  bool result = false;
  for (byte i = 0; i < str.length(); i++)
  {
    if (isDigit(str.charAt(i))) {
      result = true;
    } else {
      result = false;
      break;
    }
  }
  return result;
}
