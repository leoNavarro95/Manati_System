
#include "Arduino.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>

#define OLED_DEBUG 1//no definir para cancelar el debugin por la OLED de la placa HELTEC

#ifdef OLED_DEBUG
#include "SSD1306Wire.h" //libreria de la OLED
#define SDA 4
#define SCL 15
#define RST 16
SSD1306Wire display(0x3c, SDA, SCL);
#endif
bool conectado = false; // false = no conectado a wifi  true = conectado a wifi
bool offline = false;   //se activa al decidir que no se va a conectar
bool WifiAp = true;    //si está en falso se pone en modo Station(STA), sino en AP
TaskHandle_t xTaskLCDHandle = NULL;
TaskHandle_t xTaskTanqueHandle = NULL;
TaskHandle_t xTaskWIFIHandle = NULL;

//Archivos locales en /include path
#include "config.h" // Posee datos de la red defaults que se deben sustituir por los gravados en la SPIFFS
#include "TanqueUtil.hpp"
#include "Wifi_Utils.hpp"
#include "Server.hpp"
#include "JSON_SPIFF.hpp" //maneja el archivo config.json con todas las configuraciones reescribe a config.h
#include "API.hpp"
#include "WebSockets.hpp"
#include "Utils_AWS.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"

void TaskLCD(void *pvParameters);        //nucleo 0
void TaskWifiServer(void *pvParameters); //tarea de gestion de la wifi nucleo 1
void TaskTanque(void *pvParameters);     //controla todo lo del tanque nucleo 0

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

bool inicializado = false;

//###################LCD######################################
void TaskLCD(void *pvParameters) // This is a task.
{
  (void)pvParameters;

  lcd.begin(16, 2);
  // create a new character
  lcd.createChar(0, bombaOff);
  // create a new character
  lcd.createChar(1, bombaOn);

  lcd.setCursor(0, 0);
  lcd.print("Tank System");
  lcd.setCursor(0, 1);
  lcd.print("Inicializando");
  lcd.display();

  vTaskDelay(pdMS_TO_TICKS(2000));

  Serial.println("Inicializando...");
  // while (!inicializado)
  // { //mientras no se haya inicializado el array para el promediador del nivel se queda en este estado
  //   vTaskDelay(pdMS_TO_TICKS(300));
  // }
  Serial.println("OK");

  bool Flag = false;
  if(!WifiAp){
      while (!Flag)
    {
      if (!conectado)
      {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Conectando...");
        lcd.display();
        vTaskDelay(pdMS_TO_TICKS(200));
      }
      if (!(conectado) && offline)
      {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Modo Offline");
        lcd.display();
        vTaskDelay(pdMS_TO_TICKS(3000));
        Flag = true; //para salir del while
      }
      if (conectado)
      {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Conectado");
        
        if (WifiAp)//si está en modo AP
        {
          lcd.setCursor(0, 1);
          lcd.print(WiFi.softAPIP()); //
        }
        else//si está en STA
        {
          lcd.setCursor(0, 1);
          lcd.print(WiFi.localIP());
        }

        lcd.display();
        vTaskDelay(pdMS_TO_TICKS(3000));
        Flag = true; //para salir del while
      }
    }
  }
  

  //ya se inicializo el sensor ultrasónico
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Flujo:");
  lcd.setCursor(9, 0);
  lcd.print("L/min");
  lcd.setCursor(0, 1);
  lcd.print("Nivel:");
  lcd.setCursor(11, 1);
  lcd.print("%");
  lcd.display();

  for (;;) // A Task shall never return or exit.
  {
    lcd.setCursor(6, 0);
    lcd.print("  ");
    lcd.setCursor(6, 0);
    lcd.print(flowRate, 0);
    lcd.setCursor(8, 1);
    lcd.print("   ");
    lcd.setCursor(8, 1);
    lcd.print(nivel);

    if (conectado)
    {
      lcd.setCursor(13, 1);
      lcd.print("OK");
    }
    else
    {
      lcd.setCursor(13, 1);
      lcd.print("NC");
    }
    switch (estadoSensores)
    {
    case 1:
      lcd.clear();
      lcd.print("Error de sensor");
      lcd.display();
      break;
    case 2:
      lcd.clear();
      lcd.print("Error de bomba");
      lcd.display();
      break;
    }
    //si la bomba esta encendida
    if (pump_on)
    {
      lcd.setCursor(6, 1);
      lcd.write(byte(1)); //bomba encendida
    }
    else
    {
      //si la bomba esta apagada
      lcd.setCursor(6, 1);
      lcd.write(byte(0)); //bomba apagada
    }

    lcd.display();
    Serial.println("LCD running");
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

//###################Tanque Control###########################
void TaskTanque(void *pvParameters)
{
  (void)pvParameters;
  pinMode(CTROL_RELAY_GPIO, OUTPUT); //salida bomba
  digitalWrite(CTROL_RELAY_GPIO, LOW);
  pinMode(BUZZER_PIN, OUTPUT); //salida del buzzer
  pinMode(sensorPin, INPUT_PULLUP);
  attachInterrupt(sensorPin, pulseCounter, FALLING);

  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT);  // Sets the echoPin as an Input

  //inicializando el arreglo para el filtro promedidiador
  for (int thisReading = 0; thisReading < numReadings; thisReading++)
  {
    readings[thisReading] = 0;
  }
  //para evitar la demora que adiciona el filtro al inicio
  for (int i = 0; i < 20;)
  {
    distance = get_dist();
    if (distance > 0) //el for solo se incrementara mientras existan lecturas válidas
    {
      filtroProm(distance);
      i++;
    }

    vTaskDelay(pdMS_TO_TICKS(50));
  }
  inicializado = true;

  while (1)
  {
    //se obtiene el nivel
    get_level();

    Serial.println("Nivel: " + String(get_level()));
    // a partir de las lecturas de nivel se decide si encender o apagar la bomba
    control_Pump(CTROL_RELAY_GPIO);
    //delay para estabilidad de las medidas de los sensores
    vTaskDelay(pdMS_TO_TICKS(1000)); //IMPORTANTE ESTE DELAY DEBE SER SIEMPRE DE 1 SEGUNDO DEBIDO AL SENSOR DE FLUJO
    check_flujo();                   //si la bomba esta encendida revisar que exista flujo a su salida
  }
}

//###################Server Wifi##############################
void TaskWifiServer(void *pvParameters)
{
  (void)pvParameters;
  //pinMode(LED_BUILTIN, OUTPUT);
  // vTaskSuspend(xTaskTanqueHandle); //suspenden las tareas hasta que se configure la wifi
  // vTaskSuspend(xTaskLCDHandle);
#ifdef OLED_DEBUG
  pinMode(RST, OUTPUT);
  digitalWrite(RST, HIGH); // para activar la OLED 1_0_1
  vTaskDelay(pdMS_TO_TICKS(60));
  digitalWrite(RST, LOW);
  vTaskDelay(pdMS_TO_TICKS(60));
  digitalWrite(RST, HIGH); // para activar la OLED
  display.init();
  display.flipScreenVertically();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, "Inicializando");
  display.display();
#endif

  SPIFFS.begin();
  getConfig(); //obtiene la configuracion desde la SPIFFS
  myPass = String(PASSWORD);
  mySsid = String(SSID);

  if (WifiAp)
  {
    ConnectWiFi_AP(); //inicia en modo AP
  }
  else
  {
    ConnectWiFi_STA(); //inicia en modo station, se conecta a una wifi predefinida en config.h
  }
  //#########OTA#############
  initOTA();

  InitServer();
  InitWebSockets();
  // pinMode(0, INPUT_PULLUP);
  //pinMode(LED_BUILTIN, OUTPUT);

  // vTaskResume(xTaskTanqueHandle); //se inician las otras dos tareas ya que se configuró la wifi
  // vTaskResume(xTaskLCDHandle);
  // readFile(SPIFFS, "/config.json");
  while (1)
  {
    // updateGPIO("BOTON", !(digitalRead(0)));
    // updateSensorValue("3", String(1));

    if (conectado)
    {
      ArduinoOTA.handle();
    }

    //ojo hay que gestionar si hay alguien conectado para enviar los datos
    Serial.println("WIFI task running");
    uint8_t mylevel = get_level();
    actualizaNivel(mylevel);
    //Serial.println("Nivel: " + String(mylevel));
    // Serial.println(String(NIVEL_ALTO) + " " + String(NIVEL_BAJO) + " " +String(UMBRAL_FLUJO));
    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);

  // Now set up tasks to run independently.
  xTaskCreatePinnedToCore(
      TaskLCD, "Task_LCD", // A name just for humans
      1024 * 2,            // This stack size can be checked & adjusted by reading the Stack Highwater
      NULL, 1              // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
      ,
      &xTaskLCDHandle, 0);

  xTaskCreatePinnedToCore(TaskTanque, "Task_Tanque", 1024 * 2, NULL, 6, &xTaskTanqueHandle /*Handler de la tarea*/, 0);

  xTaskCreatePinnedToCore(TaskWifiServer, "Task_WIFI", 1024 * 5, NULL, 4, &xTaskWIFIHandle, 1);

  // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.
}

void loop()
{

  //Usando FreeRTOS asi que va vacio
}
//#######################################################################################################
