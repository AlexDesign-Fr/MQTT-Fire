/*
   Bibliothèque pour l'annimation de leds.
*/

#include <Adafruit_NeoPixel.h>
#define PIN_LED        D1

/*
#define LEDS_DEBUG(message) \
  Serial.print("[DEBUG:"); \
  Serial.print(__func__); \
  Serial.print("("); \
  Serial.print(__LINE__); \
  Serial.print(")]-> "); \
  Serial.println(message);
// Mode prod
*/
#define LEDS_DEBUG(message);


// Le nombre de pixels déclaré ici n'est pas important @FIXME
// Le nombre de pixels utilisé est celui qui est stocké en EEPROM, à savoir LED_COUNT
Adafruit_NeoPixel strip = Adafruit_NeoPixel(100, PIN_LED, NEO_GRB + NEO_KHZ800);

// Bibliothèque pour la simulation de feu
#include "fire.h"
NeoFire fire(strip);



// --------------------------------------------------------------------------------
// Allume les leds
//
void LED_fireStart(){
  fire.Draw();
  delay(random(150,300));
}


// --------------------------------------------------------------------------------
// Eteind le ruban de leds
//
void LED_fireStop(){
  fire.Clear();
}