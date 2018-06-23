#include <Homie.h>
#include <WEMOS_SHT3X.h>
#include <WifiUDP.h>
#include <String.h>
#include <NTPClient.h>
#include <Time.h>
#include <TimeLib.h>
#include <Timezone.h>

// Homie Firmware
#define CUSTOM_FIRMWARE_NAME "sht30shield-d1mini-sensor"
#define CUSTOM_FIRMWARE_VERSION "1.0.4"

// Define NTP properties
#define NTP_OFFSET   60 * 60      // In seconds
#define NTP_INTERVAL 60 * 1000    // In miliseconds
#define NTP_ADDRESS  "ca.pool.ntp.org"  // change this to whatever pool is closest (see ntp.org)

// Max Wifi connections retries
#define MAX_WIFI_RETRIES 3
// Max MQTT connections retries
#define MAX_MQTT_RETRIES 3

// Set up the NTP UDP client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

// SHT 30 address - Default on D1 mini shield
SHT3X sht30(0x45);

// Set up NTP data
String date;
String t;
const char * days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"} ;
const char * months[] = {"Jan", "Feb", "Mar", "Apr", "May", "June", "July", "Aug", "Sep", "Oct", "Nov", "Dec"} ;
const char * ampm[] = {"AM", "PM"} ;

// executions variables and flags
long lastTemperatureSent = 0;
int currentWifiRetries = 0;
int currentMQTTRetries = 0;

HomieSetting<long> sensorReadingInterval("interval", "Sensor reading interval");

//long deepSleepTimeInSeconds = 10; // FOR DEBUG REASONS
long deepSleepTimeInSeconds = 300; // Default value

HomieNode temperatureNode("temperature", "temperatureC");
HomieNode humidityNode("humidity", "humidityPercentage");
HomieNode timeNode("lastmqttmessage", "timestamp");

void setupHandler() {
  //Start the NTP UDP client
  timeClient.begin();
  
  // Settings
  sensorReadingInterval.setDefaultValue(300).setValidator([] (long candidate) {
    return (candidate >= 0) && (candidate <= 3600);
  });

  deepSleepTimeInSeconds = sensorReadingInterval.get();  
}

void updateAndSendDateAndTime() {
  date = "";  // clear the variables
  t = "";

  // update the NTP client and get the UNIX UTC timestamp
  timeClient.update();
  unsigned long epochTime =  timeClient.getEpochTime();

  // convert received time stamp to time_t object
  time_t local, utc;
  utc = epochTime;

  // Then convert the UTC UNIX timestamp to local time
  TimeChangeRule usEDT = {"EDT", Second, Sun, Mar, 2, -300};  //UTC - 5 hours - change this as needed
  TimeChangeRule usEST = {"EST", First, Sun, Nov, 2, -360};   //UTC - 6 hours - change this as needed
  Timezone usEastern(usEDT, usEST);
  local = usEastern.toLocal(utc);

  // now format the Time variables into strings with proper names for month, day etc
  date += days[weekday(local) - 1];
  date += ", ";
  date += months[month(local) - 1];
  date += " ";
  date += day(local);
  date += ", ";
  date += year(local);

  // format the time to 12-hour format with AM/PM and no seconds
  t += hourFormat12(local);
  t += ":";
  if (minute(local) < 10) // add a zero if minute is under 10
    t += "0";
  t += minute(local);
  t += " ";
  t += ampm[isPM(local)];

  // Display and send the date and time
  Homie.getLogger() << "Local date: " << date << " Local time" << t << endl;
  timeNode.setProperty("date").setRetained(true).send(date);
  timeNode.setProperty("time").setRetained(true).send(t);
}

boolean readAndSendSensorData () {
  // Return false by default
  boolean result = false;

  // If sensor available
  if (sht30.get() == 0 ) {

    // Send Temperature
    float temperature = sht30.cTemp; // Celsius Temperature
    Homie.getLogger() << "Temperature: " << temperature << " °C" << endl;
    temperatureNode.setProperty("degrees").setRetained(true).send(String(temperature));

    // Send Temperature
    float hum = sht30.humidity; // Relative humidity
    Homie.getLogger() << "Humidity: " << hum << " %" << endl;
    humidityNode.setProperty("relative").setRetained(true).send(String(hum));
    result = true;
  }
  return result;
}

void onHomieEvent(const HomieEvent& event) {
  switch (event.type) {
    case HomieEventType::WIFI_CONNECTED:
      Homie.getLogger() << "Wifi connected - Resetting retry number which was " << currentWifiRetries << endl;
      currentWifiRetries = 0;
      break;
    case HomieEventType::WIFI_DISCONNECTED:
      if (currentWifiRetries < MAX_WIFI_RETRIES) {
        currentWifiRetries++;
        Homie.getLogger() << "Wifi disconnected - #" << currentWifiRetries << endl;
      } else {
        Homie.getLogger() << "Too much wifi retries - Ready to sleep" << endl;
        Homie.prepareToSleep();
      }
      break;
    case HomieEventType::MQTT_READY:
      Homie.getLogger() << "MQTT connected - Resetting retry number which was " << currentMQTTRetries << endl;
      currentMQTTRetries = 0;      
      break;
    case HomieEventType::MQTT_DISCONNECTED:
      if (currentMQTTRetries < MAX_MQTT_RETRIES) {
        currentMQTTRetries++;
        Homie.getLogger() << "MQTT disconnected - #" << currentMQTTRetries << endl;
      } else {
        Homie.getLogger() << "Too much MQTT connection retries - Ready to sleep" << endl;
        Homie.prepareToSleep();
      }
      break;
    case HomieEventType::READY_TO_SLEEP:      
      Homie.getLogger() << "Let's go sleeping" << endl;
      ESP.deepSleep(deepSleepTimeInSeconds * 1000000);
  }
}

void loopHandler() {

  if (millis() - lastTemperatureSent >= deepSleepTimeInSeconds * 1000L || lastTemperatureSent == 0) {
    // Read sensor data at end of setup and close connection again
    if (readAndSendSensorData()) {
      updateAndSendDateAndTime();
    }
    lastTemperatureSent = millis();

    // Go to sleep as soon as transmission is complete
    Homie.getLogger() << "Preparing for deep sleep (" << deepSleepTimeInSeconds << " seconds)" << endl;

    // Prevent loop to start again when getting into Deep Sleep Mode
    Homie.prepareToSleep();
  }
}



void setup() {
  Serial.begin(115200);
  Homie.getLogger() << endl << endl;

  Homie_setFirmware(CUSTOM_FIRMWARE_NAME, CUSTOM_FIRMWARE_VERSION);
  Homie.setSetupFunction(setupHandler);
  Homie.setLoopFunction(loopHandler);

  // Increase mqtt client Keep Alive. To avoid broker disconnection issues, should be more than the Deep Sleep Interval.
  // Let's put it 2x the Deep Sleep Interval
  Homie.getLogger() << "Force MQTT Connection keep alive at 2x DS Interval (" << sensorReadingInterval.get() * 2 << " seconds)" << endl;
  Homie.getMqttClient().setKeepAlive(sensorReadingInterval.get() * 2);

  Homie.onEvent(onHomieEvent);
  Homie.setup();
}

void loop() {  
  Homie.loop();
}
