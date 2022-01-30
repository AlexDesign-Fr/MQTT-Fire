/**
    Bibliothèque pour avoir une mise à jour Over The Air d'un code source.
    Utilisation :
      Dans le setup, rajouter OTA_setup();
      Dans la loop, rajouter OTA_doUpdate();
    Fonctionnement :
      Toutes les OTA_TimerInSecond secondes, le programme va vérifier qu'il y a une mise à jour sur le serveur.
      Si une mise à jour existe, elle est téléchargée et installé et l'ESP reboot.
      S'il n'y a pas de mise à jour, le serveur le dit et on ne fait rien.

    Avant toutes upload de ce script dans un Arduino, il faut executer un script Python de récupération de certificats qui
    se trouve sur le repot https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WiFi/examples/BearSSL_CertStore/certs-from-mozilla.py
*/
/*
// Mode debug
#define OTA_DEBUG(message) \
  Serial.print("[DEBUG:"); \
  Serial.print(__func__); \
  Serial.print("("); \
  Serial.print(__LINE__); \
  Serial.print(")]-> "); \
  Serial.println(message);
// Mode prod (sans aucune traces)
*/
#define OTA_DEBUG(message);

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
char outputBuffer[100];  // Pour les messages à afficher
char macAdresse[12];

// Fait un serveur de mise à jour local dans l'ESP
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266WebServer.h>
ESP8266WebServer OTA_HttpServer(80);
ESP8266HTTPUpdateServer OTA_httpUpdater;
const char* OTA_update_username = "ESPadmin";
const char* OTA_update_password = "admin";
const char* OTA_update_path = "/firmware";
boolean g_BOO_UpdateFirmware = true;


const String firmwareUrlMiseAJour   = "https://update.alex-design.fr/MQTT-leds-color/update.php";
const String fileSystemUrlMiseAJour = "https://update.alex-design.fr/MQTT-leds-color/updateFS.php";
//const String firmwareUrlMiseAJour = "http://192.168.0.32:9090/MQTT-leds-color/update.php";  <- Ne peut pas fonctionner car il est en http, et on veut du https
// Define global variable to know if upate is available
long OTA_UpdateTimer;
const int OTA_TimerInSecond = 60 * 10; // every 10 minute


// Utilisation d'un certificat ------------------------------------------
// Pour mettre à jour l'heure (obligatoire avec un certificat https)
#include <time.h>
// A single, global CertStore which can be used by all
// connections.  Needs to stay live the entire time any of
// the WiFiClientBearSSLs are present.
#include <CertStoreBearSSL.h>
BearSSL::CertStore certStore;
#include <FS.h>
#include <LittleFS.h>
char versionLitteFS[10] = "0.0.0";


// Define a wifi client
ESP8266WiFiMulti WiFiMulti;

/**
   ----------------------------------------------------------------------------------------------------------------
   Callback lorsque la maj OTA démarre
*/
void _update_started() {
  OTA_DEBUG("CALLBACK:  HTTPS update process started");
}

/**
   ----------------------------------------------------------------------------------------------------------------
   Callback lorsque la maj OTA est terminée
*/
void _update_finished() {
  OTA_DEBUG("CALLBACK:  HTTPS update process finished. Reboot");
}

/**
   ----------------------------------------------------------------------------------------------------------------
   Callback lorsque la maj OTA est en cours
*/
void _update_progress(int cur, int total) {
  sprintf(outputBuffer, "CALLBACK:  HTTPS update process at %d of %d bytes...", cur, total);
  OTA_DEBUG( outputBuffer );
}


/**
   ----------------------------------------------------------------------------------------------------------------
   Callback lorsque la maj OTA a plantée
*/
void _update_error(int err) {
  sprintf(outputBuffer, "CALLBACK:  HTTPS update fatal error code %d\n", err);
  OTA_DEBUG( outputBuffer );

  if (err == -103) {
    OTA_DEBUG("           Please allow me, I am ");
    OTA_DEBUG(WiFi.macAddress());
    OTA_DEBUG(WiFi.localIP());
  }

  if ( err == 0 ) {
    OTA_DEBUG("La mise à jour du firmware via OTA n'a pas marché, on stop !!!!!!!!!!!!");
    g_BOO_UpdateFirmware = false;
  }
}




/**
   ----------------------------------------------------------------------------------------------------------------
   Set time via NTP, as required for x.509 validation
   ----------------------------------------------------------------------------------------------------------------
*/
void OTA_setClock() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");  // UTC
  OTA_DEBUG("OTA Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    yield();
    delay(500);
    Serial.print(F("."));
    now = time(nullptr);
  }
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);

  OTA_DEBUG("Current time: ");
  OTA_DEBUG(asctime(&timeinfo));
}



/**
   ----------------------------------------------------------------------------------------------------------------
   Initialisation d'une connexion wifi à l'aide des settings de wifimanager en eeprom.
   ----------------------------------------------------------------------------------------------------------------
*/
void OTA_setup() {

  /* Mise en place d'un serveur pour uploader directement un binaire */
  OTA_httpUpdater.setup(&OTA_HttpServer, OTA_update_path, OTA_update_username, OTA_update_password);
  OTA_HttpServer.begin();
  sprintf(outputBuffer, "OTA HTTPUpdateServer ready! Open http://%d.%d.%d.%d%s in your browser and login with username '%s' and password '%s'\n",
          WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3], OTA_update_path, OTA_update_username, OTA_update_password);
  OTA_DEBUG(outputBuffer);
  MQTT_publishDebug(String(outputBuffer));




  // Test la version du file system littleFS en place
  if ( !LittleFS.begin()) {
    OTA_DEBUG("Il n'y a pas de file system little FS installé, lecture impossible !");

  } else {
    // Get version of the certificat in LittleFS (frome file version.txt
    File file = LittleFS.open("/version.txt", "r");
    if ( file ) {
      while (file.available()) {
        strcpy(versionLitteFS, file.readString().c_str() );
      }
      file.close();
    }
    
    // Récupération des CA stockés dans LittleFS pour les certificats SSL
    int numCerts = certStore.initCertStore(LittleFS, PSTR("/certs.idx"), PSTR("/certs.ar"));
    OTA_DEBUG("Number of CA certs read: ");
    OTA_DEBUG(numCerts);
    if (numCerts == 0) {
      OTA_DEBUG(F("No certs found. Did you run certs-from-mozill.py and upload the LittleFS directory before running?"));
    }
  }
  OTA_DEBUG("Numéro de version de littleFS : " + String(versionLitteFS) );


  // Création de la connection Wifi à partir du SSID et PWD sauvé par wifimanager
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(wifiManager.getWiFiSSID(true).c_str(), wifiManager.getWiFiPass(true).c_str());


  /* We will request a firmware update in OTA_TimerInSecond secondes */
  OTA_UpdateTimer = (OTA_TimerInSecond * 1000);
}





/**
   ----------------------------------------------------------------------------------------------------------------
   Va voir sur l'URL si une mise à jour du firmware est disponible. Si c'est le cas,
   la télécharge et met à jour le firmeware.
   ----------------------------------------------------------------------------------------------------------------
*/
boolean OTA_doUpdate() {
  // Si on a pas besoin de faire de mise à jour (ou ça c'est planté la dernière fois)
  if ( !g_BOO_UpdateFirmware ) {
    return false;
  }
    
  // Fait tourner le serveur http sur l'ESP
  // Lorsqu'on se connect à ce serveur, il est possible d'uploader un firmware ou un filesystem
  OTA_HttpServer.handleClient();

  // Check is this is the time to check a new update
  delay( 1 ); // Wait 1 milliseconde
  if (OTA_UpdateTimer > 0 ) {
    OTA_UpdateTimer--;
    return false;
  }

  if (WiFiMulti.run() == WL_CONNECTED) {
    // WiFiClient client; // Client simple (incompatioble en https )

    // Mise à jour de l'heure via un serveur NTP
    OTA_setClock();

    // Récupération du certificat SSL pour la connexion https
    BearSSL::WiFiClientSecure client; // Client securise
    bool mfln = client.probeMaxFragmentLength(fileSystemUrlMiseAJour, 443, 1024);  // server must be the same as in ESPhttpUpdate.update()
    if (mfln) {
      client.setBufferSizes(1024, 1024);
    }
    client.setCertStore(&certStore);

    // Add optional callback notifiers
    ESPhttpUpdate.onStart(_update_started);
    ESPhttpUpdate.onEnd(_update_finished);
    ESPhttpUpdate.onProgress(_update_progress);
    ESPhttpUpdate.onError(_update_error);
    ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);


    // Get the mac adresse in a char
    snprintf(macAdresse, 12, "%06X", (uint32_t)ESP.getChipId() );


    // Try to update the filesystem
    OTA_DEBUG(fileSystemUrlMiseAJour + "?chipID=" + String(macAdresse) );
    t_httpUpdate_return ret = ESPhttpUpdate.updateFS(client, fileSystemUrlMiseAJour + "?chipID=" + String(macAdresse), versionLitteFS);
    if (ret == HTTP_UPDATE_OK) {
      OTA_DEBUG("Update FileSystem Successfully");
    }


    // Try to update the firmware
    OTA_DEBUG(firmwareUrlMiseAJour + "?chipID=" + String(macAdresse) );
    OTA_DEBUG("Version firmware :" + firmwareActualVersion );

    ret = ESPhttpUpdate.update(client, firmwareUrlMiseAJour + "?chipID=" + String(macAdresse), firmwareActualVersion);
    switch (ret) {
      case HTTP_UPDATE_FAILED:

        sprintf(outputBuffer, "HTTP_UPDATE_FAILED Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
        OTA_DEBUG( outputBuffer );

        OTA_UpdateTimer = (OTA_TimerInSecond * 1000);
        break;


      case HTTP_UPDATE_NO_UPDATES:
        OTA_DEBUG("No new update available");

        OTA_UpdateTimer = (OTA_TimerInSecond * 1000);
        break;

      default:
        break;
    }

    return true;
  }
  return false;
}
