/**
 * The MySensors Arduino library handles the wireless radio link and protocol
 * between your home built sensors/actuators and HA controller of choice.
 * The sensors forms a self healing radio network with optional repeaters. Each
 * repeater and gateway builds a routing tables in EEPROM which keeps track of the
 * network topology allowing messages to be routed to nodes.
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
 * DESCRIPTION
 *
 * déclaration de tous les noeuds du systeme pour initialiser tous les skeches 
 * Mysensors et s'y retrouver dans ls messages 
 *
 */


// capteur pour node chauffe-eau solaire
#define CHILD_CAPT_TEMP_BAS 1 // temperature en bas du chauffee-eau
#define CHILD_CAPT_TEMP_HAUT 2 // temperature en haut du chauffee-eau
#define CHILD_CAPT_TEMP_PANN_SOL 3 // temperature pannea solaire
