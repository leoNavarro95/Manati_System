#include "EEPROM.h"

#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

void initOTA()
{
  // Port defaults to 3232
  // ArduinoOTA.setPort(3232);

  // Hostname defaults to esp3232-[MAC]
  // ArduinoOTA.setHostname("myesp32");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA
      .onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
          type = "sketch";
        else // U_SPIFFS
          type = "filesystem";

        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
        Serial.println("Start updating " + type);
      })
      .onEnd([]() {
        Serial.println("\nEnd");
      })
      .onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
      })
      .onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR)
          Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR)
          Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR)
          Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR)
          Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR)
          Serial.println("End Failed");
      });

  ArduinoOTA.begin();
  // ArduinoOTA.handle(); //esta definicion debe ejecutarse a un periodo menor de 5 segundos en la tarea de gestion de la conexion wifi
}

#define EEPROM_SIZE 64
int addr = 1;

String mySsid;
String myPass;
// byte intentos = 0;
void ConnectWiFi_STA(bool useStaticIP = false)
{
  if (!EEPROM.begin(EEPROM_SIZE))
  {
    Serial.println("failed to initialise EEPROM");
  }
  int intentos = byte(EEPROM.read(addr));

  if (intentos >= 6)
  { //si hay cualquier dato random en la memoria o está limpia (255)
    EEPROM.write(addr, byte(0));
    EEPROM.commit();
    intentos = byte(EEPROM.read(addr));
  }
  int cont = 0;
  Serial.println("");
  WiFi.mode(WIFI_STA);
  Serial.println("Conectándose: " + String(intentos));

  WiFi.begin("MyAp", "12345678Leo");
  if (useStaticIP)
    WiFi.config(ip, gateway, subnet);
  while (WiFi.status() != WL_CONNECTED)
  {
    vTaskDelay(pdMS_TO_TICKS(100));
    Serial.print('.');
    cont++;
    if (cont >= 20)
    {
      intentos++;
      EEPROM.write(addr, intentos); //se guarda en la eeprom virtual para poder recuperarlo cuando bootee
      EEPROM.commit();
      Serial.println("Error de conexion. Reiniciando...");
      ESP.restart();
    }
    if (intentos >= 5)
    {
      conectado = false;//no conectado
      offline=true;
      Serial.println("Error de conexion. Se desabilitará la pagina web embebida...");
      vTaskResume(xTaskTanqueHandle); //se inician las otras dos tareas para que funcionen
      vTaskResume(xTaskLCDHandle);
      intentos = 0;                 //se reinicia la bandera
      EEPROM.write(addr, intentos); //se guarda en la eeprom virtual para poder recuperarlo cuando bootee
      EEPROM.commit();
      vTaskDelay(pdMS_TO_TICKS(500));
      vTaskSuspend(xTaskWIFIHandle);
    }
  }

  intentos = 0;                 //se reinicia la bandera
  EEPROM.write(addr, intentos); //se guarda en la eeprom virtual para poder recuperarlo cuando bootee
  EEPROM.commit();
  conectado = true;

  Serial.println("");
  Serial.print("Iniciado STA:\t");
  Serial.println(SSID);
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());
}

void ConnectWiFi_AP(bool useStaticIP = false)
{

  WiFi.mode(WIFI_AP);
  while (!WiFi.softAP(mySsid.c_str(), myPass.c_str())) //while(!WiFi.softAP(ssid, password))
  {

    Serial.println("Error en configurar la AP");
    while (1)
    {
      vTaskDelay(pdMS_TO_TICKS(100));
    }
  }
  if (useStaticIP)
    WiFi.softAPConfig(ip, gateway, subnet);
  Serial.println("");
  Serial.print("Iniciado AP:\t");
  Serial.println(mySsid);
  Serial.print("IP address:\t");
  Serial.println(WiFi.softAPIP());
  Serial.println("");
}