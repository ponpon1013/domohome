/**
 * The MySensors Arduino library handles the wireless radio link and protocol
 * between your home built sensors/actuators and HA controller of choice.
 * The sensors forms a self healing radio network with optional repeaters. Each
 * repeater and gateway builds a routing tables in EEPROM which keeps track of the
 * network topology allowing messages to be routed to nodes.
 *
 * Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
 * Copyright (C) 2013-2015 Sensnology AB
 * Full contributor list: https://github.com/mysensors/Arduino/graphs/contributors
 *
 * Documentation: http://www.mysensors.org
 * Support Forum: http://forum.mysensors.org
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 *******************************
 *
 * REVISION HISTORY
 * Version 1.0 - Henrik EKblad
 * Contribution by tekka,
 * Contribution by a-lurker and Anticimex, 
 * Contribution by Norbert Truchsess <norbert.truchsess@t-online.de>
 * Contribution by Ivo Pullens (ESP8266 support)
 * 
 * DESCRIPTION
 * The EthernetGateway sends data received from sensors to the WiFi link. 
 * The gateway also accepts input on ethernet interface, which is then sent out to the radio network.
 *
 * VERA CONFIGURATION:
 * Enter "ip-number:port" in the ip-field of the Arduino GW device. This will temporarily override any serial configuration for the Vera plugin. 
 * E.g. If you want to use the defualt values in this sketch enter: 192.168.178.66:5003
 *
 * LED purposes:
 * - To use the feature, uncomment WITH_LEDS_BLINKING in MyConfig.h
 * - RX (green) - blink fast on radio message recieved. In inclusion mode will blink fast only on presentation recieved
 * - TX (yellow) - blink fast on radio message transmitted. In inclusion mode will blink slowly
 * - ERR (red) - fast blink on error during transmission error or recieve crc error  
 * 
 * See http://www.mysensors.org/build/esp8266_gateway for wiring instructions.
 * nRF24L01+  ESP8266
 * VCC        VCC
 * CE         GPIO4          
 * CSN/CS     GPIO15
 * SCK        GPIO14
 * MISO       GPIO12
 * MOSI       GPIO13
 * GND        GND
 *            
 * Not all ESP8266 modules have all pins available on their external interface.
 * This code has been tested on an ESP-12 module.
 * The ESP8266 requires a certain pin configuration to download code, and another one to run code:
 * - Connect REST (reset) via 10K pullup resistor to VCC, and via switch to GND ('reset switch')
 * - Connect GPIO15 via 10K pulldown resistor to GND
 * - Connect CH_PD via 10K resistor to VCC
 * - Connect GPIO2 via 10K resistor to VCC
 * - Connect GPIO0 via 10K resistor to VCC, and via switch to GND ('bootload switch')
 * 
  * Inclusion mode button:
 * - Connect GPIO5 via switch to GND ('inclusion switch')
 * 
 * Hardware SHA204 signing is currently not supported!
 *
 * Make sure to fill in your ssid and WiFi password below for ssid & pass.
 */

#include <EEPROM.h>
#include <SPI.h>
#include <ArduinoOTA.h>
#include <DallasTemperature.h>

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS D3

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);


#include <Ticker.h>
Ticker TempTick;

// Enable debug prints to serial monitor
//#define MY_DEBUG 

// Use a bit lower baudrate for serial prints on ESP8266 than default in MyConfig.h
#define MY_BAUD_RATE 9600

// Enables and select radio type (if attached)
#define MY_RADIO_NRF24
//#define MY_RADIO_RFM69

#define MY_GATEWAY_ESP8266

#define MY_ESP8266_SSID "Livebox-09DA"
#define MY_ESP8266_PASSWORD "oWwNZy6GHd3hsRrxAk"

// Set the hostname for the WiFi Client. This is the hostname
// it will pass to the DHCP server if not static.
// #define MY_ESP8266_HOSTNAME "sensor-ota-gateway"

// Enable UDP communication
//#define MY_USE_UDP

// Enable MY_IP_ADDRESS here if you want a static ip address (no DHCP)
//#define MY_IP_ADDRESS 192,168,178,87

// If using static ip you need to define Gateway and Subnet address as well
//#define MY_IP_GATEWAY_ADDRESS 192,168,178,1
//#define MY_IP_SUBNET_ADDRESS 255,255,255,0

// The port to keep open on node server mode 
#define MY_PORT 5003      

// How many clients should be able to connect to this gateway (default 1)
#define MY_GATEWAY_MAX_CLIENTS 2

// Controller ip address. Enables client mode (default is "server" mode). 
// Also enable this if MY_USE_UDP is used and you want sensor data sent somewhere. 
//#define MY_CONTROLLER_IP_ADDRESS 192, 168, 178, 68

// Enable inclusion mode
//#define MY_INCLUSION_MODE_FEATURE

// Enable Inclusion mode button on gateway
// #define MY_INCLUSION_BUTTON_FEATURE
// Set inclusion mode duration (in seconds)
//#define MY_INCLUSION_MODE_DURATION 60 
// Digital pin used for inclusion mode button
//#define MY_INCLUSION_MODE_BUTTON_PIN  3 

 
// Flash leds on rx/tx/err
// #define MY_LEDS_BLINKING_FEATURE
// Set blinking period
// #define MY_DEFAULT_LED_BLINK_PERIOD 300

// Led pins used if blinking feature is enabled above
//#define MY_DEFAULT_ERR_LED_PIN 16  // Error led pin
//#define MY_DEFAULT_RX_LED_PIN  16  // Receive led pin
//#define MY_DEFAULT_TX_LED_PIN  16  // the PCB, on board LED

#if defined(MY_USE_UDP)
  #include <WiFiUDP.h>
#else
  #include <ESP8266WiFi.h>
#endif

//on utilise la platine ESP8266 avec registre a decalge
#define USE_PLATINE

// fichier en tête gerant les numero de neoud du systeme
#include "domohome.h"


//nombre d'actionneurs
#define NB_ACTIONNEUR 4

#define RESOLUTION 9 // temperature resolution
#define COMPARE_TEMP 0 // Send temperature only if changed? 1 = Yes 0 = No
#define MAX_ATTACHED_DS18B20 16 // maximum de capteur sur la ligne

//unsigned long SLEEP_TIME = 60000; // Sleep time between reads (in milliseconds)


// variable glaobales ---------------------------------------------------------------------------------------

//variable temperatureCapteurSolaire provenant du noeud Capteusolaire (numero neoud dans fichier domohome.h)
float TempCaptSolaire=15.0; // 15.0 valeur par defaut
// variable tempDernierEnvoiTempCaptSolaire qui sait la derniere fois qu'on a eu l'info de TempCaptSolaire

float lastTemperature[MAX_ATTACHED_DS18B20]; // tableau qui stocke les temprature

//nbre de capteur temperature detecte
int8_t numSensors=0;//nombre de capteur trouves

// tableau des adresse des capteur de temperature
DeviceAddress Thermometer[MAX_ATTACHED_DS18B20];

//tableau  de valeur sur les capteur de temperature relevées
float tabTemp[MAX_ATTACHED_DS18B20];

long tempDernierEnvoiTempCaptSolaire;

volatile int tabActionneurIndex[NB_ACTIONNEUR];
volatile bool tabActionneurState[NB_CAPTEUR];




//fin variable globale -------------------------------------------------------------------------------------
// contante temps à ne pas dépasser sans reception de TempCaptSolaire
#define TEMP_MAX_RECEPTION_CAPTEUR_SOLAIRE 1800000.0 //1800000=30 mn

#include <MySensors.h>

void SenTAddressController(DeviceAddress deviceAddress,int numero)
{
  char toSend[256];
  char val[256];
  strcpy (toSend,"");
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) strcat(toSend,"0");;
    itoa(deviceAddress[i],val,16);
    strcat (toSend,val);
    strcat(toSend," ");
   }
  MyMessage msgInfo(S_TEXT_TEMP_CHAUFFE_EAU_BAS + numero,V_TEXT);
  send(msgInfo.set(toSend));
}

void getTemp()
{
  int i;
   
  sensors.requestTemperatures(); // Send the command to get temperatures

  for (i=0;i<numSensors;i++)
  {
    SenTAddressController(Thermometer[i],i);
    tabTemp[i]= sensors.getTempC(Thermometer[i]);
    MyMessage msgTemp(S_TEMP_CHAUFFE_EAU_BAS + i,V_TEMP);
    send(msgTemp.set(tabTemp[i],1));
   }
}

#ifdef MY_DEBUG 
// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
    Serial.print(" ");
  }
  Serial.println("");
}
#endif

bool TestTempEnvoieCaptSoalire()
{
   if((millis() - tempDernierEnvoiTempCaptSolaire) > TEMP_MAX_RECEPTION_CAPTEUR_SOLAIRE)
   {
     return false;
   }
   else
   {
    return true;
   }
}

void RegistreWrite(int numberToDisplay)
{
  // on met le nombre sous 8 bit
  numberToDisplay=numberToDisplay & 255;
  // the LEDs don't change while you're sending in bits:
    digitalWrite(clockPin, LOW);
    digitalWrite(latchPin, LOW);
    // shift out the bits:
    shiftOut(dataPin, clockPin, LSBFIRST, numberToDisplay);  

    //take the latch pin high so the LEDs will light up:
    digitalWrite(latchPin, HIGH);
}

void before()
{
  // Startup up the OneWire library
  sensors.begin();
  sensors.setResolution(RESOLUTION);
}

void setup() { 
 
  #ifdef MYDEBUG  
  ArduinoOTA.onStart([]() {
   Serial.println("ArduinoOTA start"); 
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nArduinoOTA end");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("OTA Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  #endif
  ArduinoOTA.begin();
#ifdef MYDEBUG  
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
#endif
  //initilaisation de tempDernierEnvoiTempCaptSolaire
  //tempDernierEnvoiTempCaptSolaire=millis();

  //set pins to output so you can control the shift register
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);

   // on met le regsitre à 1 pour tout etéindre ( etat haut ferme)
  RegistreWrite(255);
    
  //initilaisation indexActionneur
  tabActionneurIndex[0]=S_REMPLIR;
  tabActionneurIndex[1]=S_VIDER;
  tabActionneurIndex[2]=S_CIRCULER;
  tabActionneurIndex[3]=S_CHAUFFER;
  
  //initilaisation des actionneur à 0
  int i;
  for (i=0;i<NB_ACTIONNEUR;i++)
  {
    tabActionneurState[tabActionneurIndex[i]]=false;
  }

  change=false;
  data=255;

  // mise en ràoute du timer Ticker
  TempTick.attach(60, getTemp);
}

void presentation() {
    int i;
  // Present locally attached sensors here


  // Fetch the number of attached temperature sensors  
  numSensors = sensors.getDeviceCount();
  #ifdef MYDEBUG  
  Serial.print("nbre capteur trouvés:");
  Serial.println(numSensors);
  #endif
  // Present all sensors to controller
   for (i=0;i<numSensors;i++)
  {
    
    if (!sensors.getAddress(Thermometer[i], i)) 
    {
      #ifdef MYDEBUG  
      Serial.print("Unable to find address for Device");
      Serial.println(i);
      #endif
    }
    else
    {
      present(S_TEMP_CHAUFFE_EAU_BAS + i, S_TEMP);
      tabTemp[i]=15.0; // valeur par défaut de temperature
      #ifdef MYDEBUG  
      printAddress(Thermometer[i]);
      #endif
      sensors.setResolution(Thermometer[i], RESOLUTION);
    }
    
  }
 
  // senseur virtuel qui indique si la derniere recpetion de capteur solaire est trop ancienne
  present(S_TEMP_CAPT_SOLAIRE_VALIDE, S_TEMP);

  //actionneur---------------------------------------------
  
  for (i=0;i<NB_ACTIONNEUR;i++)
  {
    present(tabActionneurIndex[i], S_BINARY);
  }
  // fin présensataton actionneur--------------------------

  // presentation texte
  present(S_TEXT_TEMP_CHAUFFE_EAU_BAS, S_INFO);
  present(S_TEXT_TEMP_CHAUFFE_EAU_HAUT, S_INFO);

}


void loop() {
  // Send locally attech sensors data here
  ArduinoOTA.handle();

  //lance la vérification de la validité de tempcaptsolaire
  // si valide on envoie au controller vali sinon non-valide
  //MyMessage msgValTempCaptSolaire(S_TEMP_CAPT_SOLAIRE_VALIDE,V_STATUS);
  //send(msgValTempCaptSolaire.set(TestTempEnvoieCaptSoalire()));

  
  // on renvoi au controleur l'etat des actionneur
   int i;
   
  int datawrite; // data à ecrire sur data 
  if (change)
  {
    #ifdef MYDEBUG  
    Serial.println("change detecté");
    Serial.print("data AVANT:");
    Serial.println(data,BIN);
    #endif
    
    //on actionne les actionneur  à actionner
    data=0;
    for (i=NB_ACTIONNEUR-1;i>=0;i--)
    {
      #ifdef MYDEBUG  
      Serial.print("i:");
      Serial.print(i);
      Serial.print(" etat actionneur:");
      Serial.print(tabActionneurState[tabActionneurIndex[i]]);
      Serial.print("data avant");
      Serial.print(data,BIN);
      #endif
      datawrite=tabActionneurState[tabActionneurIndex[i]] << i;
      data=data | datawrite;
      #ifdef MYDEBUG  
      Serial.print("data apres ecriture");
      Serial.print(data,BIN);
      #endif
    }
    data = data ^255 ;
    #ifdef MYDEBUG  
    Serial.println("change detecté");
    Serial.print("data apres:");
    Serial.println(data,BIN);
    #endif
    // on inverse data car l'état haut est à la masse
    
    RegistreWrite(data);
  
    for (i=0;i<NB_ACTIONNEUR;i++)
    {
      MyMessage msgActioneur(tabActionneurIndex[i],V_STATUS);
      send(msgActioneur.set(tabActionneurState[tabActionneurIndex[i]]));
    }
    change=false;
  }
 }


void receive(const MyMessage &message){
  #ifdef MYDEBUG  
  Serial.println("Message reçu");
  Serial.print("envoyeur:"); Serial.println(message.sender);
  Serial.print("type:"); Serial.println(message.type);
  Serial.print("senseur:"); Serial.println(message.sensor);
  Serial.print("valeur:"); Serial.println(message.getFloat());
  #endif
  if (message.sender==201 && message.type==V_TEMP) // message venant de capteurSolaire
  {
    TempCaptSolaire=message.getFloat();
    //on reinitialise la nouvelle date de réception de capteur solaire
    tempDernierEnvoiTempCaptSolaire=millis();
  }

//message venant du controlleur
   if (message.type==V_STATUS) // message venant de capteurSolaire
  {
    // on récupere l'actionneur concerné et on le change d'etat
    int i;
    for (i=0;i<NB_ACTIONNEUR;i++)
    {
     if (message.sensor==tabActionneurIndex[i])
     {
      tabActionneurState[tabActionneurIndex[i]]=message.getBool();
      change=true;
     }
    }
  }
  
}




