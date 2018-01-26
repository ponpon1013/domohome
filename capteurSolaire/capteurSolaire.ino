//include domohome librairy containing node mapping
#include "domohome.h"

// Enable and select radio type attached
#define MY_RADIO_NRF24

#define MY_NODE_ID NODE_CAPTEUR_SOLAIRE  // numéro du nœud

// Set blinking period (in milliseconds)
#define MY_DEFAULT_LED_BLINK_PERIOD 300

#define MY_DEFAULT_ERR_LED_PIN 4
#define MY_DEFAULT_TX_LED_PIN 5
#define MY_DEFAULT_RX_LED_PIN 6


// Include the libraries we need
#include <SPI.h>
#include <MySensors.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 3

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

#define RESOLUTION 9 // temperature resolution
#define COMPARE_TEMP 0 // Send temperature only if changed? 1 = Yes 0 = No
#define MAX_ATTACHED_DS18B20 16 // maximum de capteur sur la ligne
float lastTemperature[MAX_ATTACHED_DS18B20]; // tableau qui stocke les temprature
int8_t numSensors=0;//nombre de capteur trouves
unsigned long SLEEP_TIME = 60000; // Sleep time between reads (in milliseconds)

// Initialize temperature message
MyMessage msg(0,V_TEMP);

void before()
{
  // Startup up the OneWire library
  sensors.begin();
  sensors.setResolution(RESOLUTION);
}


void setup(void)
{
  
}

void presentation() {
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("Temperature Panneau Solaire", "1.1");

  // Fetch the number of attached temperature sensors  
  numSensors = sensors.getDeviceCount();

  // Present all sensors to controller
  present(0, S_TEMP);
}

/*
 * Main function, get and show the temperature
 */

void loop(void)
{ 
  // call sensors.requestTemperatures() to issue a global temperature 
  // request to all devices on the bus
  //Serial.print("Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  
  float temperature = sensors.getTempCByIndex(0);
   
   // Only send data if temperature has changed and no error
    #if COMPARE_TEMP == 1
    if (lastTemperature[0] != temperature && temperature != -127.00 && temperature != 85.00) {
    #else
    if (temperature != -127.00 && temperature != 85.00) {
    #endif
 
      // Send in the new temperature to controller
     send(msg.setSensor(0).set(temperature,1).setDestination(0));

     // Send in the new temperature to gateway which is also sensors chauffe-eau-solaire
     
     
     wait(MY_DEFAULT_LED_BLINK_PERIOD); //on attend que les diodes s'éteignent
     lastTemperature[0]=temperature;
       
    }
   
   sleep(SLEEP_TIME);
}

