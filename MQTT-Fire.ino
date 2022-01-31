// --------------------------------------------------------------------------------
// Programme permettant :
// - d'allumer/éteindre une barre de LEDS (branchée sur la PIN D1 ).
// - de changer la couleur des LEDS.
// - de modifier l'intensité des LEDS.
// - de lancer une animation des LEDS.
//
// Le programme se branche en WIFI sur un broker MQTT et réagi au topics :
// lumiere/<DeviceID> [ON|OFF]                : Allumage de la barre de LEDS.
// --------------------------------------------------------------------------------

// Pour une mise en prod, ne pas oublier de mettre writeToEEPROM = false
const String firmwareActualVersion = "1.0.2";
const boolean writeToEEPROM = false;  // Si = true, écrit en EEPROM et affiche les traces de debug de wifimanager (default = false)




#include <ESP8266HTTPClient.h>
#include <WiFiManager.h>
WiFiManager wifiManager;

// Define memory size we want to define (1 bytes / caract) for EPPROM storage
#define EEPROM_LEDS_SIZE 3
#define EEPROM_DEVICEID_SIZE 15


// Customisation du nom du module ESP
#define HOSTNAME "ESP8266-"  // Pour la connection Wifi (doit être unique)
char HostName[16];

// Pour répondre au topic MQTT [fire]
char DeviceID[EEPROM_DEVICEID_SIZE] = "fire";  // N'est pris en compte que si writeToEEPROM = true (car sinon on lit la valeur provenant de l'EEPROM)



// EEPROM ( pour stockage du nombre de leds )
// 30:83:98:82:6a:6f Support Casque -> 5 leds
int LED_COUNT = 5;  // N'est pris en compte que si writeToEEPROM = true (car sinon on lit la valeur provenant de l'EEPROM)
#include "EEPROM.h"


// LEDS
boolean g_BOO_Animation = true;
#include "leds.h"


// Définition d'une structure pouvant stocker le message provenant de MQTT
#include "MQTT.h"


// Over The Air
#include "OTA.h"


// *********************************************************************************
void setup() {
  // initialisation de la liaison série (pour le moniteur) .........................
  Serial.begin(115200);
  delay(5000);  // On attend que le port serie soit initialisé
  Serial.println();
  Serial.flush();
  Serial.println("***********************************************************************************");
  Serial.println("OK, let's go **********************************************************************");
  Serial.println("Version firmware :" + String(firmwareActualVersion));
  MQTT_publishDebug("Version firmware :" + String(firmwareActualVersion));


  // Lecture du nombre de leds dans l'EEPROM ........................................
  // si writeToEEPROM = true, on sauve la valeur lue dans la globale LED_COUNT
  EEPROM_Start();


  // initialisation de la liaison WIFI ..............................................
  /* Si la connexion échoue, on lance un Access Point (AP) qui est visible dans les réseaux WIFI
      Il faut alors se connecter avec un smarthpone sur l'AP pour configurer le Wifi, le NodeMCU
      reboot et se connect avec le SSID et mot de passe saisie.
  */
  snprintf(HostName, 16, HOSTNAME "%06X", (uint32_t)ESP.getChipId());  // Concaténation du HOSTNAME avec la fin de l'adresse MAC
  wifiManager.setDebugOutput(writeToEEPROM);                           // false ->Pour ne plus avoir le mot de passe WIFI qui s'affiche.
  wifiManager.autoConnect(HostName, "123456789");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("HOSTNAME: ");
  Serial.println(HostName);
  wifiManager.setHostname(HostName);


  // Connection Wifi pour l'OTA ....................................................
  OTA_setup();


  // Create a MQTT client ..........................................................
  MQTT_setup();


  // Initialisation des leds .....................................................
  strip.begin();  // INITIALIZE NeoPixel strip object
  strip.show();   // Turn OFF all pixels ASAP
  LED_fireStart();


  Serial.println("************************** Tout est initialise");
  MQTT_publishDebug("--------------------------- Tout est initialise");
}







// **********************************************************************************************************
// **********************************************************************************************************
void loop() {
  // On écoute le serveur OTA
  OTA_doUpdate();


  // Test si la connection Wifi existe toujours ...................................
  if (WiFi.status() != WL_CONNECTED) {
    // Si on est déconnecté on tente de se reconnecter automatiquement avec les anciens settings.
    wifiManager.autoConnect();
  }



  // Test si la connection MQTT est toujours valide ..............................
  if (!clientMQTT.connected()) {
    Serial.println("OUPS, on est plus connecté au server MQTT--------------------------");

    MQTT_connect();

    // On reboot
    //ESP.restart();
  }
  clientMQTT.loop();

  if( g_BOO_Animation ) {
    LED_fireStart();
  }


  // Traitement des Messages MQTT ...................................................
  // Tout est fait dans  MQTT_callback()
}
