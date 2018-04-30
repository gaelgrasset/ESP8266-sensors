#include <Homie.h>

#define CUSTOM_FIRMWARE_NAME "arcadebuttons-led-control"
#define CUSTOM_FIRMWARE_VERSION "1.0.0"
#define DEBOUNCE_TIME_MILLIS 60 //Timer needes to debounce the momentary switch (prevent another misreading during this time)
#define RED_BUTTON_PIN D1
#define YELLOW_BUTTON_PIN D2
#define GREEN_BUTTON_PIN D3
#define BLUE_BUTTON_PIN D4
#define WHITE_BUTTON_PIN D5
#define POWER_BUTTON_PIN D6

// One node representing the buttons. Possible values are the button colors connected to the NodeMCU chip :
// red, yellow, green, blue, white, and power (for the second white).
HomieNode buttonNode("button", "button");

// Init of the variables used in the loop
long lastButtonPressed = 0; // Use for debouncing
int redButtonLastState = 0;
int redButtonCurrentState = 0;
int yellowButtonLastState = 0;
int yellowButtonCurrentState = 0;
int greenButtonLastState = 0;
int greenButtonCurrentState = 0;
int blueButtonLastState = 0;
int blueButtonCurrentState = 0;
int whiteButtonLastState = 0;
int whiteButtonCurrentState = 0;
int powerButtonLastState = 0;
int powerButtonCurrentState = 0;

void setupHandler() {
  // Setup the pins
  pinMode(RED_BUTTON_PIN, INPUT);
  pinMode(YELLOW_BUTTON_PIN, INPUT);
  pinMode(GREEN_BUTTON_PIN, INPUT);
  pinMode(BLUE_BUTTON_PIN, INPUT);
  pinMode(WHITE_BUTTON_PIN, INPUT);
  pinMode(POWER_BUTTON_PIN, INPUT);
}

void readButtonsState() {
  redButtonCurrentState = digitalRead(RED_BUTTON_PIN);
  yellowButtonCurrentState = digitalRead(YELLOW_BUTTON_PIN);
  greenButtonCurrentState = digitalRead(GREEN_BUTTON_PIN);
  blueButtonCurrentState = digitalRead(BLUE_BUTTON_PIN);
  whiteButtonCurrentState = digitalRead(WHITE_BUTTON_PIN);
  powerButtonCurrentState = digitalRead(POWER_BUTTON_PIN);
}

void loopHandler() {
  // Need some loop time to debounce the arcade buttons
  if (millis() - lastButtonPressed >= DEBOUNCE_TIME_MILLIS * 1000L || lastButtonPressed == 0) {
    boolean buttonHasBeenPressed = false;
    readButtonsState();

    // RED BUTTON
    if (redButtonCurrentState != redButtonLastState) {
      // if the state has changed, let's react
      if (redButtonCurrentState == HIGH) {
        Homie.getLogger() << "RED Button pushed" << endl;
        // send the message over MQTT that the button has been pressed
        buttonNode.setProperty("pressed").send("red");
      } else {
        Homie.getLogger() << "RED Button released" << endl;
      }
      redButtonLastState = redButtonCurrentState;
      buttonHasBeenPressed = true;
    }

    // GREEN BUTTON
    if (greenButtonCurrentState != greenButtonLastState) {
      // if the state has changed, let's react
      if (greenButtonCurrentState == HIGH) {
        Homie.getLogger() << "GREEN Button pushed" << endl;
        // send the message over MQTT that the button has been pressed
        buttonNode.setProperty("pressed").send("green");
      } else {
        Homie.getLogger() << "GREEN Button released" << endl;
      }
      greenButtonLastState = greenButtonCurrentState;
      buttonHasBeenPressed = true;
    }

    // YELLOW BUTTON
    if (yellowButtonCurrentState != yellowButtonLastState) {
      // if the state has changed, let's react
      if (yellowButtonCurrentState == HIGH) {
        Homie.getLogger() << "YELLOW Button pushed" << endl;
        // send the message over MQTT that the button has been pressed
        buttonNode.setProperty("pressed").send("yellow");
      } else {
        Homie.getLogger() << "YELLOW Button released" << endl;
      }
      yellowButtonLastState = yellowButtonCurrentState;
      buttonHasBeenPressed = true;
    }

    // BLUE BUTTON
    if (blueButtonCurrentState != blueButtonLastState) {
      // if the state has changed, let's react
      if (blueButtonCurrentState == HIGH) {
        Homie.getLogger() << "BLUE Button pushed" << endl;
        // send the message over MQTT that the button has been pressed
        buttonNode.setProperty("pressed").send("blue");
      } else {
        Homie.getLogger() << "BLUE Button released" << endl;
      }
      blueButtonLastState = blueButtonCurrentState;
      buttonHasBeenPressed = true;
    }

    // WHITE BUTTON
    if (whiteButtonCurrentState != whiteButtonLastState) {
      // if the state has changed, let's react
      if (whiteButtonCurrentState == HIGH) {
        Homie.getLogger() << "WHITE Button pushed" << endl;
        // send the message over MQTT that the button has been pressed
        buttonNode.setProperty("pressed").send("white");
      } else {
        Homie.getLogger() << "WHITE Button released" << endl;
      }
      whiteButtonLastState = whiteButtonCurrentState;
      buttonHasBeenPressed = true;
    }    

    // POWER BUTTON
    if (powerButtonCurrentState != powerButtonLastState) {
      // if the state has changed, let's react
      if (powerButtonCurrentState == HIGH) {
        Homie.getLogger() << "POWER Button pushed" << endl;
        // send the message over MQTT that the button has been pressed
        buttonNode.setProperty("pressed").send("power");
      } else {
        Homie.getLogger() << "POWER Button released" << endl;
      }
      powerButtonLastState = powerButtonCurrentState;
      buttonHasBeenPressed = true;
    }

    if (buttonHasBeenPressed) {
      lastButtonPressed = millis();
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial << endl << endl;
  Homie_setFirmware(CUSTOM_FIRMWARE_NAME, CUSTOM_FIRMWARE_VERSION);
  Homie.setSetupFunction(setupHandler).setLoopFunction(loopHandler);

  // Increase mqtt client Keep Alive. To avoid broker disconnection issues, should be more than the Deep Sleep Interval.
  // Let's put it 2x the Deep Sleep Interval
  Homie.getLogger() << "Force MQTT Connection keep alive at 1 hour..." << endl;
  Homie.getMqttClient().setKeepAlive(3600);
  
  Homie.setup();
}

void loop() {
  Homie.loop();
}
