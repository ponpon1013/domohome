/**
   The MySensors Arduino library handles the wireless radio link and protocol
   between your home built sensors/actuators and HA controller of choice.
   The sensors forms a self healing radio network with optional repeaters. Each
   repeater and gateway builds a routing tables in EEPROM which keeps track of the
   network topology allowing messages to be routed to nodes.

   Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
   Copyright (C) 2013-2015 Sensnology AB
   Full contributor list: https://github.com/mysensors/Arduino/graphs/contributors

   Documentation: http://www.mysensors.org
   Support Forum: http://forum.mysensors.org

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   version 2 as published by the Free Software Foundation.

 *******************************

   REVISION HISTORY
   Version 1.0: Nicolas Ponsonnaille

   DESCRIPTION
   tets communication entre deux noeuds


*/

// Enable debug prints
#define MY_DEBUG

// Enable and select radio type attached
#define MY_RADIO_NRF24
//#define MY_RADIO_RFM69
//#define MY_RS485

#define MY_REPEATER_FEATURE

// définition des numéro des capteurs
#define NODE_CAPTEUR_TEST1 51
#define NODE_CAPTEUR_TEST2 52
#define CHILD_SENSOR_ID_SEND 1
#define MY_NODE_ID NODE_CAPTEUR_TEST2

#include <SPI.h>
#include <MySensors.h>

bool metric = true;
float temperatureLu = 10.0;
float temperatureEnvoyee = 12.0;

// Initialize temperature message
MyMessage msg(0, V_TEMP);

void presentation() {
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("Temperature test communication capteur test1", "1.0");
  present(0, S_TEMP);
  present(CHILD_SENSOR_ID_SEND, S_TEMP);
}


void loop()
{
  // on demande la valeur à l'autre noeud
  request(CHILD_SENSOR_ID_SEND, V_TEMP, NODE_CAPTEUR_TEST1);
  
  
  // on envoie les valeurs des fils au controlleur
  send(msg.setSensor(0).set(temperatureLu, 1));
  send(msg.setSensor(CHILD_SENSOR_ID_SEND).set(temperatureEnvoyee, 1));
  
  wait(60000);
}

void receive(const MyMessage &message)
{
  if(message.type == V_TEMP && message.sender != 0)
  {
      Serial.print("Node "); Serial.print(message.sender); 
      Serial.print(" Commandtype: "); Serial.print(message.getCommand());
      Serial.println(" valeur: ");Serial.print(message.getFloat());
  }
}










