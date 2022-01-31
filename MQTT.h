// MQTT client
#include <PubSubClient.h>
#define mqtt_broker "192.168.0.11"
#define MQTT_user ""
#define MQTT_password ""

/*
#define MQTT_DEBUG(message) \
  Serial.print("[DEBUG:"); \
  Serial.print(__func__); \
  Serial.print("("); \
  Serial.print(__LINE__); \
  Serial.print(")]-> "); \
  Serial.println(message);
// Mode prod
*/
#define MQTT_DEBUG(message);


// DEFINITION DES TOPICS POUR CE MODULE -------------------------------------------
char topic_lumiere[8 + EEPROM_DEVICEID_SIZE];
char topic_lumiere_color[8 + 6 + EEPROM_DEVICEID_SIZE];
char topic_lumiere_bright[8 + 11 + EEPROM_DEVICEID_SIZE];
char topic_lumiere_anim[8 + 10 + EEPROM_DEVICEID_SIZE];



WiFiClient espClient;
PubSubClient clientMQTT(espClient); // Definition du client MQTT


char g_CHAR_messageBuff[100];




// --------------------------------------------------------------------------------
// Envoie un message sur le canal de debug MQTT.
//
void MQTT_publishDebug(String message){
  strcpy( g_CHAR_messageBuff, "/hardware/debug/MQTT-leds-color/"); // Initialisation de <g_CHAR_messageBuff> avec lce topic
  strcat( g_CHAR_messageBuff, HostName);         // Concatenation de l'ID du hostname
 
  // Publicaiton du message
  clientMQTT.publish(g_CHAR_messageBuff,message.c_str() );
}


// --------------------------------------------------------------------------------
// Envoi les mesures ("data") passées en paramètre au brocker MQTT.
// L'envoie se fait sous la forme :
// sensor/temperature/<sensorID>/<value>
// avec "sensor/temperature qui est dans le parametre p_CHAR_topic
//
// @param moduleID : L'identifiant du thermometre défini dans FIBARO
// @param data : la valeur de la mesure.
// @p_CHAR_topic : un char pointant sur la chaine contenant le nom du topic dans lequel on veut publier
//
void MQTT_publishDataToMQTT(String moduleID, String value, char *p_CHAR_topic) {
  // Creation du topic (on rajoute un / suivi de l'ID du sensor)
  String topic = "/" + moduleID;
  char buff[20];
  topic.toCharArray(buff, 20); // On met le topic dans la variable char buff

  // Construction du char contenant le topic pour ce module
  strcpy( g_CHAR_messageBuff, p_CHAR_topic); // Initialisation de <g_CHAR_messageBuff> avec ler topic qui est passé en paramètre
  strcat( g_CHAR_messageBuff, buff);         // Concatenation de temperature_topic + buff

  // Publication de la temperature dans le topic
  MQTT_DEBUG("Publication d'un message sur le topic :");
  MQTT_DEBUG(g_CHAR_messageBuff);

  clientMQTT.publish(g_CHAR_messageBuff, String(value).c_str() );
}




// --------------------------------------------------------------------------------
// Reconnexion au serveur MQTT
//
void MQTT_connect() {
  //Boucle jusqu'à obtenir une reconnexion
  while (!clientMQTT.connected()) {
    Serial.print("Connexion au serveur MQTT...");
    MQTT_publishDebug(" Connexion au serveur MQTT...");

    // ON arrive à se conecter au brocker MQTT
    if (clientMQTT.connect(HostName, MQTT_user, MQTT_password)) {
      MQTT_DEBUG("OK");
      MQTT_publishDebug(" OK");

    // Connection au brocker MQTT ratée
    } else {
      Serial.print("KO, erreur : ");
      Serial.println(clientMQTT.state());
      MQTT_DEBUG(" On attend 5 secondes avant de recommencer");
      MQTT_publishDebug( "... Connection impossible. On attend 5 secondes avant de recommencer\nErreur connection = " + String(clientMQTT.state()) );
      delay(5000);
    }
  }

  // Souscription aux topics
  clientMQTT.subscribe("lumiere/#");
  MQTT_publishDebug("Abonnement au topic MQTT lumiere/#");
}





// --------------------------------------------------------------------------------
// Déclenche les actions à la réception d'un message MQTT.
// lumiere/portal [ON|OFF]               : Allumage de la barre de LEDS.
//
void MQTT_callback(char* topic, byte* payload, unsigned int length) {

  // create character buffer with ending null terminator (string)
  char message[100];
  int i = 0;
  for (i = 0; i < length; i++) {
    message[i] = payload[i];
  }
  message[i] = '\0';

  


  // Traitement des topics
  // ................................................................................
  if ( strcmp( topic, topic_lumiere ) ==0 )  {
    MQTT_DEBUG("Detection du topics :" + String( topic_lumiere ));

    if ( String( message ) == "ON") {
      MQTT_DEBUG("Allumage les leds");
      MQTT_publishDebug("MQTT_callback> Allumage les leds ");
      LED_fireStart();
      g_BOO_Animation = true;

    } else if ( String( message ) == "OFF") {
      MQTT_DEBUG("Extinction des leds");
      MQTT_publishDebug("MQTT_callback> Extinction les leds ");
      LED_fireStop();
      g_BOO_Animation = false;
    }
  }
}



// --------------------------------------------------------------------------------
// Initialisation du brocker MQTT.
//
void MQTT_setup(){
  // Création du client MQTT
  clientMQTT.setServer(mqtt_broker, 1883);  // Configuration de la connexion au serveur MQTT
  clientMQTT.setCallback(MQTT_callback);    // La fonction de callback qui est executée à chaque réception de message


  // Connection au Brocker MQTT
  MQTT_connect();


  // Construction des topcs auxquels s'abonner.
  sprintf( topic_lumiere,         "lumiere/%s", DeviceID);
}
