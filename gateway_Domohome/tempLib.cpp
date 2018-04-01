#include "Arduino.h"
#include "tempLib.h"

OneWire oneWire(ONE_WIRE_BUS);

TempCapteur::TempCapteur(){
  int i;
  lastRequest=millis(); //initialisation du compteur de temps
  
  sensors.setOneWire(&oneWire);
  sensors.begin();
  // Fetch the number of attached temperature sensors  
  numSensors = sensors.getDeviceCount();
  if (numSensors!=0){
    for (i=0;i<numSensors;i++)
    {
    //  present(S_TEMP_CHAUFFE_EAU_BAS + i, S_TEMP);
      sensors.getAddress(Thermometer[i], i);
      tabTemp[i]=15.0; // valeur par défaut de temperature
      sensors.setResolution(Thermometer[i], RESOLUTION);
    }
  }
}

void TempCapteur::getTemp(){
  int i;
  if (lastRequest>millis()) lastRequest=millis(); // cas ou millis a été réinitialisé
  
  if (millis()-lastRequest> 50000){
    lastRequest=millis();
    sensors.requestTemperatures(); // Send the command to get temperatures
    for (i=0;i<numSensors;i++)
    {
      tabTemp[i]= sensors.getTempC(Thermometer[i]);
    }
  }
}

float TempCapteur::getTemp(int capteurNumero){
  this->getTemp();
  return tabTemp[capteurNumero];
}

char * TempCapteur::getAdress(int capteurNumero){
  int i;
  char toSend[256];
  char val[256];
  
  if (capteurNumero <= numSensors)
  {
  strcpy (toSend,"");
    for (uint8_t i = 0; i < 8; i++)
    {
      // zero pad the address if necessary
      if (Thermometer[capteurNumero][i] < 16) strcat(toSend,"0");
      itoa(Thermometer[capteurNumero][i],val,16);
      strcat (toSend,val);
      strcat(toSend," ");
     }
  }
  return toSend;
}

