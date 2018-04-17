
// HOMIE Definitions
#define CUSTOM_FIRMWARE_NAME "NODEMCU-WS1821B-CONTROL"
#define CUSTOM_FIRMWARE_VERSION "1.0.0"

// FASTLED Definitions
#define DATA_PIN      5     // for Huzzah: Pins w/o special function:  #4,#5, #12, #13, #14; // #16 does not work :(
#define LED_TYPE      WS2812B
#define COLOR_ORDER   GRB
#define NUM_LEDS      150

// Program Definitions
#define MILLI_AMPS         10000     // IMPORTANT: set here the maxmilli-Amps of your power supply 5V 2A = 2000
#define FRAMES_PER_SECOND  120 // here you can control the speed. With the Access Point / Web Server the animations run a bit slower.
