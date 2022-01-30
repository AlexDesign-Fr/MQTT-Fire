/*
   Bibliothèque pour l'annimation de leds sur une bande de leds.
*/

#include <Adafruit_NeoPixel.h>
#define PIN_LED        D1


// NeoPixel brightness, 0 (min) to 255 (max)
#define BRIGHTNESS 100 // Set BRIGHTNESS to about 1/5 (max = 255)
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
Adafruit_NeoPixel strip = Adafruit_NeoPixel(500, PIN_LED, NEO_GRB + NEO_KHZ800);


struct Couleur {
  int R = 0;
  int V = 0;
  int B = 0;
};

// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// strip.Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.
void LED_colorWipe(uint32_t color, int wait) {
  for (int i = 0; i < LED_COUNT; i++) { // For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}



// --------------------------------------------------------------------------------
// Allume la led dont le numéro est passé en paramètre
// et éteind la led précédente.
void LED_AllumeLedNum( int led) {
  
  if ( led == 0 ) {
    // Eteind les LEDs de la fin du ruban
    // LED_colorWipe(strip.Color(0, 0, 0), 0);  // Noir
    strip.setPixelColor(LED_COUNT -1, strip.Color(0, 0, 0));   // Noir

  } else {
    // Eteind la LED juste derrière celle qu'il faut allumer
    strip.setPixelColor(led - 1, strip.Color(0, 0, 0));   // Noir
  }
  strip.setPixelColor(led, strip.Color(255, 255, 255)); // Blanc
  strip.show();
}




// --------------------------------------------------------------------------------
// Vérifie que la chaine est est bien un integer entre 0 et 255
// Si c'est le cas on renvoie True, False sinon
//
boolean LED_isADigit(char* s) {
  char chaine[] = "rrr";

  int i = 0;
  for (i = 0; s[i]; i++) {
    chaine[i] = s[i]; // On construit la copie de la chaine passée en parametre
  }
  chaine[i] = s[i]; // Pour ne pas oublier le \0 de la fin

  char* couleur = NULL;
  couleur = strtok(chaine, ",");  // On travail sur la copie
  while (couleur != NULL) {
    // Convertion de la chaine en integer
    // Si l'integer n'est pas compris en 0 et 255 ...
    if (atoi( couleur ) < 0 or atoi( couleur ) > 255 ) {
      // ... on a pas une couleur, on sort du test
      return false;
    }
    couleur = strtok(NULL, ",");
  }
  return true;
}


// --------------------------------------------------------------------------------
// Vérifie que la chaine est est bien une couleur du style R,V,B
// Si c'est le cas on renvoie True, False sinon
//
boolean LED_isAColor(char* s) {
  char chaine[] = "rrr,bbb,vvv";

  // On compte les virgules dans la chaine
  int i, count = 0;
  for (i = 0; s[i]; i++) {
    if (s[i] == ',') {
      count++;
    }
    chaine[i] = s[i]; // On construit la copie de la chaine passée en parametre
  }
  chaine[i] = s[i]; // Pour na pas oublier le \0 de la fin

  // on a bien 2 virgules
  if (count == 2 ) {
    char* couleur = NULL;

    couleur = strtok(chaine, ",");  // On travail sur la copie
    while (couleur != NULL) {
      // Convertion de la chaine en integer
      // Si l'integer n'est pas compris en 0 et 255 ...
      if (atoi( couleur ) < 0 or atoi( couleur ) > 255 ) {
        // ... on a pas une couleur, on sort du test
        return false;
      }
      couleur = strtok(NULL, ",");
    }
  } else {
    return false;
  }
  return true;
}

// --------------------------------------------------------------------------------
// Convertie la chaine RVB en une couleur.
// @return Color
Couleur LED_ExtractRVB(char* s) {
  // Définition d'une couleur
  Couleur c;

  char* couleur = strtok(s, ",");
  int count = 0;
  while (couleur != NULL) {
    if (count == 0) {
      c.R = atoi(couleur);
    } else if (count == 1) {
      c.V = atoi(couleur);
    } else if (count == 2) {
      c.B = atoi(couleur);
    }
    count ++;
    couleur = strtok(NULL, ",");
  }

  return c;
}



// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
void rainbow(uint8_t wait) {
  uint16_t i, j;
  for (j = 0; j < 256; j++) {
    for (i = 0; i < LED_COUNT; i++) {
      strip.setPixelColor(i, Wheel((i + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}
// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;
  for (j = 0; j < 256 * 1; j++) { // 5 cycles of all colors on wheel
    for (i = 0; i < LED_COUNT; i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / LED_COUNT) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}
//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j = 0; j < 10; j++) { //do 10 cycles of chasing
    for (int q = 0; q < 3; q++) {
      for (uint16_t i = 0; i < LED_COUNT; i = i + 3) {
        strip.setPixelColor(i + q, c);  //turn every third pixel on
      }
      strip.show();
      delay(wait);
      for (uint16_t i = 0; i < LED_COUNT; i = i + 3) {
        strip.setPixelColor(i + q, 0);      //turn every third pixel off
      }
    }
  }
}
//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  LEDS_DEBUG("LED_COUNT:"+String(LED_COUNT));
  for (int j = 0; j < 256; j++) {   // cycle all 256 colors in the wheel
    for (int q = 0; q < 3; q++) {
      for (uint16_t i = 0; i < LED_COUNT; i = i + 3) {
        strip.setPixelColor(i + q, Wheel( (i + j) % 255)); //turn every third pixel on
      }
      strip.show();
      delay(wait);
      for (uint16_t i = 0; i < LED_COUNT; i = i + 3) {
        strip.setPixelColor(i + q, 0);      //turn every third pixel off
      }
    }
  }
}



// --------------------------------------------------------------------------------
// Fait une animation sur les leds en fonction du numéro passé en paramètre.
// [1..9]
//
void LED_Animation(int num) {
  g_BOO_AnimationSeconde = false;
  LEDS_DEBUG(num);
  switch ( num ) {
    case 0:
      LED_colorWipe(strip.Color(255, 255, 255), 20); // Blanc
      break;
    case 1:
      LED_colorWipe(strip.Color(0, 0, 255), 20);  // Bleu
      break;
    case 2:
      theaterChase(strip.Color(0, 0, 255), 50);
      break;
    case 3:
      theaterChaseRainbow(50);
      break;
    case 4:
      rainbow(50);
      break;
    case 5:
      rainbowCycle(10);
      break;
    case 6:
      g_BOO_AnimationSeconde = true;
      break;
    default:
      LEDS_DEBUG("Animation inconnue ->" + String(num) );
      break;
  }
}
