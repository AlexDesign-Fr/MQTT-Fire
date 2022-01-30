/*
   Bibliothèque permettant de stocker 2 valeurs en EEPROM
   @see : https://projetsdiy.fr/esp8266-comment-lire-ecrire-effacer-eeprom/
*/
#include <EEPROM.h>


/*----------------------------------------------------------------------------
   Permet d'écrire une chaine de caractère en EEPROM à une adresse donnée.
*/
void EEPROM_writeString(char add, String data) {
  int _size = data.length();
  int i;
  for (i = 0; i < _size; i++)
  {
    EEPROM.write(add + i, data[i]);
  }
  EEPROM.write(add + _size, '\0'); //Add termination null character for String Data
  EEPROM.commit();
}


/*----------------------------------------------------------------------------
   Permet de lire une chaine de caractère en EEPROM à une adresse donnée.
*/
String EEPROM_read_String(char add){
  int i;
  char data[100]; //Max 100 Bytes
  int len=0;
  unsigned char k;
  k=EEPROM.read(add);
  while(k != '\0' && len<500)   //Read until null character
  {    
    k=EEPROM.read(add+len);
    data[len]=k;
    len++;
  }
  data[len]='\0';
  return String(data);
}

/*----------------------------------------------------------------------------
    Permet de mettre en place la gestion du nombre de leds en EEPROM.
*/
void EEPROM_Start() {
  EEPROM.begin(EEPROM_LEDS_SIZE + EEPROM_DEVICEID_SIZE + 2);
  if ( writeToEEPROM == true ) {
    Serial.println("* * * * * * * * * * * * * Stockage des valeurs en EEPROM * * * * * * * * * * * * * * ");

    // Ecriture des valeurs dans l'EEPROM
    EEPROM.put(0, LED_COUNT); // on a un entier, on peut utiliser put
    EEPROM_writeString(sizeof(LED_COUNT), DeviceID);  // on a une chaine de caractère, on doit utiliser une fonction custom
  }

  // Relecture de la valeur stockée dans l'EEPROM
  EEPROM.get(0, LED_COUNT);
  Serial.print("Nb leds (from EEPROM) LED_COUNT: "); Serial.println(LED_COUNT);

  String data = EEPROM_read_String(sizeof(LED_COUNT));  // Lecture dans une string
  data.toCharArray(DeviceID, EEPROM_DEVICEID_SIZE); // Convertion de String en char
  Serial.print("DeviceID (from EEPROM) : "); Serial.println(DeviceID);
}
