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

#define RESOLUTION 9 // temperature resolution à definir avant d'appeler tempLib.h
// Data wire is plugged into port 2 on the Arduino à definir avant d'appeler tempLib.h
#define ONE_WIRE_BUS D3
#include "tempLib.h"
#include "actionneur.h"

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

#define COMPARE_TEMP 0 // Send temperature only if changed? 1 = Yes 0 = No

//unsigned long SLEEP_TIME = 60000; // Sleep time between reads (in milliseconds)

// variable glaobales ---------------------------------------------------------------------------------------

//variable temperatureCapteurSolaire provenant du noeud Capteusolaire (numero neoud dans fichier domohome.h)
float TempCaptSolaire=15.0; // 15.0 valeur par defaut
// variable tempDernierEnvoiTempCaptSolaire qui sait la derniere fois qu'on a eu l'info de TempCaptSolaire

long tempDernierEnvoiTempCaptSolaire;

volatile int tabActionneurIndex[NB_ACTIONNEUR];
actionneur tabActionneur[NB_CAPTEUR];

TempCapteur capteur; // creation de l'objet capteur de la lib tempLib.h

//fin variable globale -------------------------------------------------------------------------------------

// position des pin des actionneurs sur le regitre à dédcalage
#define PIN_REMPLIR 3
#define PIN_VIDER 2
#define PIN_CHAUFFER 1
#define PIN_CIRCULER 0

// contante temps à ne pas dépasser sans reception de TempCaptSolaire

#define TEMP_MAX_RECEPTION_CAPTEUR_SOLAIRE 1800000.0 //1800000=30 mn

#include <MySensors.h>

void getTemp()
{
  int i;
  for (i=0;i<capteur.numSensors;i++)
  {
    //on envoie les temperatures
    MyMessage msgTemp(S_TEMP_CHAUFFE_EAU_BAS + i,V_TEMP);
    send(msgTemp.set(capteur.getTemp(i),1));

    //on envoie les adresses
    MyMessage msgTempText(S_TEXT_TEMP_CHAUFFE_EAU_BAS + i,V_TEXT);
    send(msgTempText.set(capteur.getAdress(i)));
   }
}

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
    shiftOut(dataPin, clockPin, MSBFIRST, numberToDisplay);  

    //take the latch pin high so the LEDs will light up:
    digitalWrite(latchPin, HIGH);
}

void before()
{
  // Startup up the OneWire library
  /*sensors.begin();
  sensors.setResolution(RESOLUTION);*/
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
  tabActionneur[0].setNumeroNoeud(S_REMPLIR);
  tabActionneur[1].setNumeroNoeud(S_VIDER);
  tabActionneur[2].setNumeroNoeud(S_CIRCULER);
  tabActionneur[3].setNumeroNoeud(S_CHAUFFER);
  
  tabActionneur[0].setNumeroPin(PIN_REMPLIR);
  tabActionneur[1].setNumeroPin(PIN_VIDER);
  tabActionneur[2].setNumeroPin(PIN_CIRCULER);
  tabActionneur[3].setNumeroPin(PIN_CHAUFFER);
    
  //initilaisation des actionneur à 0
  int i;
  for (i=0;i<NB_ACTIONNEUR;i++)
  {
    tabActionneur[i].state=false;
  }

  change=false;
  data=255;

  // mise en ràoute du timer Ticker
  TempTick.attach(60, getTemp);
}

void presentation() {
    int i;
  // Present locally attached sensors here

  for (i=0;i<capteur.numSensors;i++)
  {
    present(S_TEMP_CHAUFFE_EAU_BAS + i, S_TEMP);
  }
  
  // senseur virtuel qui indique si la derniere recpetion de capteur solaire est trop ancienne
  present(S_TEMP_CAPT_SOLAIRE_VALIDE, S_TEMP);

  //actionneur---------------------------------------------
  
  for (i=0;i<NB_ACTIONNEUR;i++)
  {
    present(tabActionneur[i].getNumeroNoeud(), S_BINARY);
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
    for (i=0;i<NB_ACTIONNEUR;i++)
    {
      MyMessage msgActioneur(tabActionneur[i].getNumeroNoeud(),V_STATUS);
      send(msgActioneur.set(tabActionneur[i].state));
      datawrite=tabActionneur[i].state << tabActionneur[i].getNumeroPin();
      data=data | datawrite;
    }
    // on inverse data car l'état haut est à la masse
    data = data ^255 ;
       
    RegistreWrite(data);
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
     if (message.sensor==tabActionneur[i].getNumeroNoeud())
     {
      tabActionneur[i].state=message.getBool();
      change=true;
     }
    }
  }
  
}




