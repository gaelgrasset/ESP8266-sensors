void initFastLED(void) {
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);// for WS2812 (Neopixel)
  FastLED.setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(brightness);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, MILLI_AMPS);
  //TODO on 160Hz the LED's are full white! Why?
  if(power == 1 && currentPatternIndex == patternCount - 1) {
    fill_solid(leds, NUM_LEDS, solidColor);  
  }else {
    fill_solid(leds, NUM_LEDS, CRGB::Black);  
  }
  
  FastLED.show();
}

void loadSettings(){
  FastLED.setBrightness(255);

  currentPatternIndex = 0;

  byte r = 255;
  byte g = 255;
  byte b = 255;

  solidColor = CRGB(r, g, b);
  
  power = 1;
}
