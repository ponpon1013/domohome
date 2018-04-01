#include "Arduino.h"
#include "actionneur.h"

actionneur::actionneur(){
  state=false;
}

void actionneur::setNumeroPin(int numeroIn){
  numeroPin=numeroIn;
}

void actionneur::setNumeroNoeud(int numeroIn){
  numeroNoeud=numeroIn;
}

int actionneur::getNumeroNoeud(){
  return numeroNoeud;
}

int actionneur::getNumeroPin(){
  return numeroPin;
}




