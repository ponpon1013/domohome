/* domohome include to have the same ID for all croquis

*/

// numero de noeud
#define NODE_CAPTEUR_SOLAIRE 201

//defini le nombre d'actionneur revoir le nom pour plus tard a utiliser dans tableaux index
#define NB_CAPTEUR 8

//numero des senseurs
// gateway
#define S_TEMP_CAPT_SOLAIRE_VALIDE 0
#define S_REMPLIR 1
#define S_VIDER 2
#define S_CIRCULER 3
#define S_CHAUFFER 4
#define S_REMPLIR_MANUEL 5
#define S_VIDER_MANUEL 6
#define S_CIRCULER_MANUEL 7
#define S_CHAUFFER_MANUEL 8
#define S_TEMP_CHAUFFE_EAU_BAS 9
#define S_TEMP_CHAUFFE_EAU_HAUT 10
#define S_TEXT_TEMP_CHAUFFE_EAU_BAS 11
#define S_TEXT_TEMP_CHAUFFE_EAU_HAUT 12


// si on utilise la platine ESP8266 avec un registre à decalage
// on utilise cette durective qui identifie les pin des composants
#ifdef USE_PLATINE
//Pin connected to ST_CP of 74HC595
int latchPin = D4;
//Pin connected to SH_CP of 74HC595
int clockPin = D1; //D0
////Pin connected to DS of 74HC595
int dataPin = D0; //D1
#endif

volatile bool change; // flag pour dire si un message a change qqchose
volatile int data;

