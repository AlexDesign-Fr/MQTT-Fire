# MQTT Fire #
Programme ESP8266 permettant de simuler des flammes de feux pour mettre dans des lampions. 
La mise en route et l'arret se fait via MQTT.

Le nombre de leds est paramétrable.

La partie brocker n'est pas inclus dans le présent programme.


# Fonctionnement #
## Connection Wifi ##
La connection wifi est gérée par l'extension **WIFI manager**.

Si la connexion échoue, on lance un Access Point (AP) qui est visible dans les réseaux WIFI
Il faut alors se connecter avec un smarthpone sur l'AP pour configurer le Wifi en lui saisissant son SSID et le Mot de passe, 
le NodeMCU reboot et se connecte avec le SSID et mot de passe saisie. Une fois, ces identifiants saisies, l'ESP8266 est déclaré
sur le réseau et il ne sera plus nécessaire de les saisir.

Peux importe l'adresse IP de l'ESP8266, elle n'est pas utile pour recevoir des messages MQTT (sauf si un filtrage est mis
en place au niveau du brocker, mais cette documentation ne couvre pas cette partie).

## MQTT ##
L'adresse IP du brocker MQTT doit être définie dans le fichier MQTT.h

Les messages MQTT interprétés doivent être dans les topics:

- lumiere/<DeviceID> [ON|OFF]                : Allumage de la barre de LEDS.

avec '''DeviceID''' = *fire*


## Gestion des variables ##
Du fait que l'ESP8266 se mette à jour automatiquement il est nécessaire de sauver une partie de sa 
configuration dans l'EEPROM sous peine de la perdre par écrasement du nouveau firmware lors d'une mise à jour.

Seul, le nombre de Leds et le topic MQTT sont stockés dans l'EEPROM, il ne sont donc à configurer que lors
de la première installation du programme  dans l'ESP. 
Autrement dit, Lors de le premiere installation du programme il faut mettre la variable 
writeToEEPROM = true et ensuite la remettre à false avant de mettre en production l'ESP8266.



# Mise à jour OTA #
Bien que cela soit possible il n'est pas nécessaire de brancher l'ESP8266 sur un port USB pour le mettre à jour, 
une simple connexion wifi suffi. L'ESP8266 va cycliquement vérifier si une nouvelle version du firmeware est disponible 
dans un répertoire web. Si c'est le cas elle est automatiquement downloadée et l'ESP8266 redémarre pour faire la mise à 
jour (aucune intervention utilisateur n'est nécessaire).

Si aucune connection internet n'est disponbile, les mises à jour ne peuvent pas se faire.


## Mise à jour du firmware ##
Après test et debuggage du sketch *MQTT-Fire.ino* ou de ses dépendences, il faut:

- changer la version dans le sketch ,MQTT-Fire.ino,

- Mettre le même numéro de version dans un tag de la branche master sur le repo git.

Le script jenkins (fichier *jenkinsfile*), va s'occuper de faire la compilation du sketch et mettre à jour le 
serveur web avec le binaire de la compilation.


## Mise à jour du file system litteleFS ##
Si la liste des CA doit être mise à jour, il faut les récupérer avec le script *certs-from-mozilla.py* (en 
l'executant dans le répertoire du sketch *MQTT-Fire.ino*) puis mettre à jour la variable 
*data/version.txt*, générer le binaire littleFS et mettre ce numéro dans le nom du binaire généré.

Par exemple, actuellement la version du file system est *1.0.1*, on a donc le nom de fichier :
'''MQTT-Fire.mklittlefs__1.0.1.bin'''



### OTA via HTTPS ###
L'utlisation du HTTPS nécessite un trousseau de CA pour authentifier le certificat SSL du protocol HTTPS.
Pour cela un file system (littleFS) est mis en place avec ce trousseau. Pour l'uploader dans l'ESP, il faut :
* soit le faire via l'IDE Arduino,

* soit grace au pluggin littleFS en le faisant via l'IDE Arduino,

* soit en créant le binaire du filesystem est en le mettant dans le répertoire *fs* sur le serveur.
(le binaire est construit dans le répertoire **C:/<Users>/AppData/Local/Temp/arduino_build_985853**) Ce répertoire est indiqué au début de la compilation dans l'IDE.


### Installation de littleFS dans l'IDE Arduino###

* Download the file uploader plugin for Arduino IDE. Go to this link and click on the ESP8266LittleFS-2.6.0.zip

* Décompresser le zip dans un répertoire.

* Déplacer le répertoire dans C:\Program Files (x86)\Arduino\tools/

* Relancer l'IDE Arduino.

Il doit apparaitre un menu Outils > ESP8266 LKittleFS Data uploads. En cliquant dans ce menu, le contenu du répertoire data, s'il existe dans le répertoire du sketch, est transféré dans l'ESP et un binaire est construit dans le répertoire **C:\Users\alexp\AppData\Local\Temp\arduino_build_985853**






---
title: MQTT Fire
subtitle: Allumage/ extinction de flammes sur des LEDs via messages MQTT.
lang: fre
author: Alexandre PERETJATKO
language: C (Arduino)
hardware: Node MCU ESP8266
website: https://git.alex-design.fr/Alex/MQTT-Fire.git
...