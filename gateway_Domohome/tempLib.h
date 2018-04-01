#ifndef _TEMPLIB_H
  #define _TEMPLIB_H

  #include "Arduino.h"
  #include <DallasTemperature.h>
  #include <Time.h>

  #ifndef RESOLUTION
    #define RESOLUTION 9
  #endif

  #ifndef ONE_WIRE_BUS
    #define ONE_WIRE_BUS D3
  #endif

  #define MAX_ATTACHED_DS18B20 16 // maximum de capteur sur la ligne

  class TempCapteur {
    public:
    TempCapteur();
    void getTemp(); // fonction pour obtenir la tempertaure de tous les capteurs
    float getTemp(int ); // fonction pour obtenir la tempertaure d'un capteur en particulier
    char* getAdress(int); // fonction pour obtenir l'adresse d'un capteur en particulier
    int8_t numSensors;//nombre de capteur trouves
    
    private:
    //OneWire oneWire(int8_t );
    DallasTemperature sensors;
    DeviceAddress Thermometer[MAX_ATTACHED_DS18B20];
    float tabTemp[MAX_ATTACHED_DS18B20];
    unsigned long lastRequest;
  };
  

#endif
