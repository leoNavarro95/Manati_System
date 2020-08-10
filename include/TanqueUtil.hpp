
//##############################PANTALLA DE CRISTAL LIQUIDO#############################
#include <LiquidCrystal.h>
#include <Wire.h>
const int rs = 13, en = 12, d4 = 14, d5 = 27, d6 = 26, d7 = 25;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

byte bombaOn[8] = {
    0b00100,
    0b01110,
    0b11111,
    0b00100,
    0b00100,
    0b00100,
    0b00100,
    0b11111};
byte bombaOff[8] = {
    0b11111,
    0b00100,
    0b00100,
    0b00100,
    0b00100,
    0b11111,
    0b01110,
    0b00100};

//#########################BUZZER CONTROL#############################################################
#define BUZZER_PIN 4 //Buzzer BUZZER_PIN

double demora = 0;

void buzzer_warning()
{
  for (int i = 0; i < 2; i++)
  {
    demora = 200;
    for (int i = 0; i < 1000; i++)
    {
      digitalWrite(BUZZER_PIN, HIGH);
      delayMicroseconds(demora);
      digitalWrite(BUZZER_PIN, LOW);
      delayMicroseconds(demora);
    }

    digitalWrite(BUZZER_PIN, LOW);
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}
void buzzer_finish()
{
  // put your main code here, to run repeatedly:
  for (int i = 0; i < 2; i++)
  {
    demora = 200;
    for (int i = 0; i < 1000; i++)
    {
      digitalWrite(BUZZER_PIN, HIGH);
      delayMicroseconds(demora);
      digitalWrite(BUZZER_PIN, LOW);
      delayMicroseconds(demora);
    }

    digitalWrite(BUZZER_PIN, LOW);
    vTaskDelay(pdMS_TO_TICKS(50));

    demora = 300;
    for (int i = 0; i < 1000; i++)
    {
      digitalWrite(BUZZER_PIN, HIGH);
      delayMicroseconds(demora);
      digitalWrite(BUZZER_PIN, LOW);
      delayMicroseconds(demora);
    }

    digitalWrite(BUZZER_PIN, LOW);
    vTaskDelay(pdMS_TO_TICKS(50));

    demora = 600;
    for (int i = 0; i < 1000; i++)
    {
      digitalWrite(BUZZER_PIN, HIGH);
      delayMicroseconds(demora);
      digitalWrite(BUZZER_PIN, LOW);
      delayMicroseconds(demora);
    }
    digitalWrite(BUZZER_PIN, LOW);
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

#define CTROL_RELAY_GPIO 23 //GPIO DESTINADO A CONRTROLAR EL RELAY QUE GOBIERNA LA BOMBA

//#################filtro promedio###################################
const int numReadings = 20;

double readings[numReadings]; // the readings from the analog input
int readIndex = 0;            // the index of the current reading
double total = 0;             // the running total
double average = 0;           // the average

double filtroProm(double entrada)
{
  // subtract the last reading:
  total = total - readings[readIndex];
  // read from the sensor:
  readings[readIndex] = entrada;
  // add the reading to the total:
  total = total + readings[readIndex];
  // advance to the next position in the array:
  readIndex++;

  // if we're at the end of the array...
  if (readIndex >= numReadings)
  {
    // ...wrap around to the beginning:
    readIndex = 0;
  }

  // calculate the average:
  average = total / numReadings;
  return average;
}

//#############MEDICION CON EL SENSOR ULTRASONICO#########################
#define DIST_TOPE 120 // 120 nivel maximo, medida con el tanque vacio en cm
const int trigPin = 2; //2
const int echoPin = 5; //5

float distancia = 0;
uint8_t nivel = 0; //nivel en porciento

long duration = 0;
float distance = 0;
float distanciaPromedio = 0;

float get_dist()
{
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH, 200000); //timeout de 200000 microsegundos que es 200ms

  // Calculating the distance
  //distance = (float)(duration * 0.03432 )/ 2;
  return (float)(duration * 0.03432) / 2;
}


uint8_t estadoSensores = 0; // 0 indica que los sensores estan bien, 1- indica error de ultrasonico, 2- error de flujometro o bomba
bool errorUltrasonico = false;
bool errorFlujometro = false;

//#############OBTENCION DEL NIVEL EN PORCIENTO###########################
uint8_t ultrasonic_fail = 0; //contador para determinar fallo del sensor ultrasonico

uint8_t get_level()
{
  distance = get_dist();
  //Serial.println("Distancia: " + String(distance,1));
  if (distance <= 0.5){
    ultrasonic_fail++; //se incrementa el contador
    // Serial.println("Fail");
  }
    
    

  if (ultrasonic_fail == 5)
  { //si llega a 5 veces concecutivas con lecturas a cero es que algo anda mal con el sensor
    digitalWrite(CTROL_RELAY_GPIO, LOW);
    errorUltrasonico = true;//error sensor ultrasonico
    estadoSensores = 1;
    
    while (1)
    {
      // vTaskSuspend(NULL);
      Serial.println("Fallo del ultrasonico");
      buzzer_warning();
      vTaskDelay(pdMS_TO_TICKS(3000));
    }
  }

  if (distance > 0.5)
  { //descarta errores del sensor

    ultrasonic_fail = 0; //se resetea el contador

    distanciaPromedio = filtroProm(distance);

    nivel = DIST_TOPE - distanciaPromedio;
    nivel = map(nivel, 0, DIST_TOPE - 30, 0, 100); //90 seria el nivel maximo por seguridad que del sensor (30cm)
    // Serial.println("    Nivel " + String(nivel));
  }
  return nivel;
}

//#####################ALGORITMO PARA CONTROL AUTOMATICO DE BOMBA#################
bool llenando = true;
int bombaFail_counter = 0;
//OJO ESTOS NIVELES ESTAN EN PORCIENTO



bool aviso = true;
bool pump_on = false;

void control_Pump(unsigned int ControlPin)
{

  if (nivel <= NIVEL_BAJO && llenando == true)
  {
    
    digitalWrite(ControlPin, HIGH);
    pump_on = true;
  }
  if (nivel <= NIVEL_ALTO && llenando == true)
  {
    digitalWrite(ControlPin, HIGH);
    pump_on = true;
    // digitalWrite(ControlPin, LOW);
    // pump_on = false;
  }

  if (nivel >= NIVEL_ALTO)
  {
    digitalWrite(ControlPin, LOW);
    llenando = false;
    pump_on = false;

    if (aviso)
    {                  //bandera para que suene una sola vez
      buzzer_finish(); //solo hace falta que suene una sola vez
      aviso = false;
    }
    bombaFail_counter = 0; //se resetea
  }
  if (nivel <= NIVEL_BAJO && llenando == false)
  {
    digitalWrite(ControlPin, HIGH);
    pump_on = true;
    aviso = true;
  }
}
//##############Sensor de Flujo #################################################################
byte sensorPin = 18;
float calibrationFactor = 7; //7.5;
volatile byte pulseCount;

float flowRate = 0;

unsigned long oldTime = 0;
/*
  Insterrupt Service Routine
*/
void IRAM_ATTR pulseCounter()
{
  // Increment the pulse counter
  pulseCount++;
}


void check_flujo()
{

  if (pump_on)
  {
    if ((millis() - oldTime) > 1000) // Only process counters once per second
    {

      detachInterrupt(sensorPin);
      flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor; //flujo en litros por minuto
      oldTime = millis();

      // Reset the pulse counter so we can start incrementing again
      pulseCount = 0;

      // Enable the interrupt again now that we've finished sending output
      attachInterrupt(sensorPin, pulseCounter, FALLING);
    }
    // se chequea que el flujo no sea menor que 5
    if (int(flowRate) <= UMBRAL_FLUJO)
    { //no esta bombeando eficientemente o no lo esta haciendo
      buzzer_warning();
      bombaFail_counter++;
      Serial.println("Error flujo: " + String(bombaFail_counter));
      if (bombaFail_counter > 5)
      {
        digitalWrite(CTROL_RELAY_GPIO, LOW);
        pump_on = false;
        errorFlujometro = true;//error de flujometro o bomba sin bombear
        estadoSensores = 2;
        while (1)
        {
          //vTaskSuspend(NULL);
          buzzer_warning();
          vTaskDelay(pdMS_TO_TICKS(3000));
        }
      }
    }
    Serial.println("Flujo: " + String(flowRate));
  }
  else
    flowRate = 0;

}
